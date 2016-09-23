#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"
#include "utils.h"

void GaussSeidel(double ** u_previous, double ** u_current, int X_min, int X_max, int Y_min, int Y_max, double omega) {
	int i,j;
	for (i=X_min;i<X_max;i++)
		for (j=Y_min;j<Y_max;j++)
			u_current[i][j]=u_previous[i][j]+(u_current[i-1][j]+u_previous[i+1][j]+u_current[i][j-1]+u_previous[i][j+1]-4*u_previous[i][j])*omega/4.0;
}


int converge_limits(double ** u_previous,double ** u_current,int i_min,int i_max,int j_min,int j_max){
	int i,j;
	for (i=i_min;i<i_max;i++)
		for (j=j_min;j<j_max;j++)
			if (fabs(u_current[i][j]-u_previous[i][j])>e) return 0;
	return 1;


}
 struct timeval tts,ttf,tcs,tcf,tcomms,tcommf;   //Timers: total-tts,ttf, computation-tcs,tcf

int main(int argc, char ** argv) {
    int rank,size;
    int global[2],local[2]; //global matrix dimensions and local matrix dimensions (2D-domain, 2D-subdomain)
    int global_padded[2];   //padded global matrix dimensions (if padding is not needed, global_padded=global)
    int grid[2];            //processor grid dimensions
    int i,j,t;
    int global_converged=0,converged=0; //flags for convergence, global and per process
    MPI_Datatype dummy;     //dummy datatype used to align user-defined datatypes in memory
    double omega; 			//relaxation factor - useless for Jacobi

   
    double ttotal=0,tcomp=0,tcomm=0,total_time,comp_time,comm_time;
    
    double ** U, ** u_current, ** u_previous, ** swap; //Global matrix, local current and previous matrices, pointer to swap between current and previous
    

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    //----Read 2D-domain dimensions and process grid dimensions from stdin----//
  if (argc!=5) {
        fprintf(stderr,"Usage: mpirun .... ./exec X Y Px Py");
        exit(-1);
    		}
    else {
        global[0]=atoi(argv[1]);
        global[1]=atoi(argv[2]);
        grid[0]=atoi(argv[3]);
        grid[1]=atoi(argv[4]);
    }
	 
	   //----Create 2D-cartesian communicator----//
	//----Usage of the cartesian communicator is optional----//

    MPI_Comm CART_COMM;         //CART_COMM: the new 2D-cartesian communicator
    int periods[2]={0,0};       //periods={0,0}: the 2D-grid is non-periodic
    int rank_grid[2];           //rank_grid: the position of each process on the new communicator
    MPI_Cart_create(MPI_COMM_WORLD,2,grid,periods,0,&CART_COMM);    //communicator creation
    MPI_Cart_coords(CART_COMM,rank,2,rank_grid);	                //rank mapping on the new communicator

    //----Compute local 2D-subdomain dimensions----//
    //----Test if the 2D-domain can be equally distributed to all processes----//
    //----If not, pad 2D-domain----//
    int flag[2],pad_size[2];
    pad_size[0]=0;
    pad_size[1]=0;
    for (i=0;i<2;i++) {
        if (global[i]%grid[i]==0) {
            local[i]=global[i]/grid[i];
            global_padded[i]=global[i];
			flag[i]=0;
        }
        else {
			
            local[i]=(global[i]/grid[i])+1;
            global_padded[i]=local[i]*grid[i];
			pad_size[i]=global_padded[i]-global[i];
			flag[i]=1;
        }
    }
	
	if (local[0]<=pad_size[0] || local[1]<=pad_size[1])
		{
		//if (rank==0){
		printf("Cannot divide equally the resources--BYE \n");
		//} 
		MPI_Finalize();
		exit(-1);
		}
	
	//Initialization of omega
    omega=2.0/(1+sin(3.14/global[0]));

    //----Allocate global 2D-domain and initialize boundary values----//
    //----Rank 0 holds the global 2D-domain----//
    if (rank==0) {
        U=allocate2d(global_padded[0],global_padded[1]);   
        init2d(U,global[0],global[1]);
    }

    //----Allocate local 2D-subdomains u_current, u_previous----//
    //----Add a row/column on each size for ghost cells----//

    u_previous=allocate2d(local[0]+2,local[1]+2);
    u_current=allocate2d(local[0]+2,local[1]+2);   
       
    //----Distribute global 2D-domain from rank 0 to all processes----//
         
 	//----Appropriate datatypes are defined here----//
	/*****The usage of datatypes is optional*****/
    
    //----Datatype definition for the 2D-subdomain on the global matrix----//

    MPI_Datatype global_block;
    MPI_Type_vector(local[0],local[1],global_padded[1],MPI_DOUBLE,&dummy);
    MPI_Type_create_resized(dummy,0,sizeof(double),&global_block);
    MPI_Type_commit(&global_block);

    //----Datatype definition for the 2D-subdomain on the local matrix----//

    MPI_Datatype local_block;
    MPI_Type_vector(local[0],local[1],local[1]+2,MPI_DOUBLE,&dummy);
    MPI_Type_create_resized(dummy,0,sizeof(double),&local_block);
    MPI_Type_commit(&local_block);

    //----Rank 0 defines positions and counts of local blocks (2D-subdomains) on global matrix----//
    int * scatteroffset, * scattercounts;
    if (rank==0) {
        scatteroffset=(int*)malloc(size*sizeof(int));
        scattercounts=(int*)malloc(size*sizeof(int));
        for (i=0;i<grid[0];i++)
            for (j=0;j<grid[1];j++) {
                scattercounts[i*grid[1]+j]=1;
                scatteroffset[i*grid[1]+j]=(local[0]*local[1]*grid[1]*i+local[1]*j);
            }
    }


    //----Rank 0 scatters the global matrix----//
    
    double * initaddr;
    if (rank==0)    initaddr=&(U[0][0]);
    MPI_Scatterv(initaddr,scattercounts,scatteroffset,global_block,&(u_previous[1][1]),1,local_block,0,MPI_COMM_WORLD);
    MPI_Scatterv(initaddr,scattercounts,scatteroffset,global_block,&(u_current[1][1]),1,local_block,0,MPI_COMM_WORLD);
    if (rank==0)
        free2d(U,global_padded[0],global_padded[1]);

     int ii,jj;
     int count=0;
     int north, south, east, west;
     MPI_Datatype column;
     MPI_Type_vector(local[0]+2,1,local[1]+2,MPI_DOUBLE,&column);
     MPI_Type_commit(&column);
     for (ii=0;ii<grid[0];ii++){
	 for (jj=0;jj<grid[1];jj++)
	 {
		if (rank==count)
		{
		if (ii-1>=0)  north=1; //up
		else north=0;

	 	if (jj-1>=0)  west=1; //left
        	else west=0;

	 	if (ii+1<grid[0])  south=1; //down
        	else south=0;

	 	if (jj+1<grid[1])  east=1; //right
        	else east=0;
	 	}
	count++;
	}
 	}
	

    int i_min,i_max,j_min,j_max;
	
	
    	if (north==1 && west==1 && east==1 && south==1)
   	{	
        i_min=1;
        i_max=local[0];
        j_min=1;
        j_max=local[1];
    	}

    	if (north==0)
	{
	i_min=2;
	i_max=local[0];
	j_min=1;
	j_max=local[1];
	
	}
	
	if (south==0)
	{
		if (flag[0]==0)
		{
		i_min=1;
		i_max=local[0]-1;
		j_min=1;
		j_max=local[1];	
		}
		else 
		{
		i_min=1;
		i_max=local[0]-1-pad_size[0];
		j_min=1;
		j_max=local[1];	
		}	
	
	}

	if (east==0)
	{
		if (flag[1]==0)
		{
		i_min=1;
		i_max=local[0];
		j_min=1;
		j_max=local[1]-1;	
		}
		else
		{
		i_min=1;
		i_max=local[0];
		j_min=1;
		j_max=local[1]-1-pad_size[1];		
		}
	
	}
	if (west==0)
	{
	i_min=1;
	i_max=local[0];
	j_min=2;
	j_max=local[1];
	
	}
	if (north==0 && west==0)
	{
	i_min=2;
	i_max=local[0];
	j_min=2;
	j_max=local[1];
	}

	if (south==0 && west==0)
	{
		if (flag[0]==0)
		{
		i_min=1;
		i_max=local[0]-1;
		j_min=2;	
		j_max=local[1];
		}

		if (flag[0]==1)
		{
		i_min=1;
		i_max=local[0]-1-pad_size[0];
		j_min=2;	
		j_max=local[1];
		}
	}
	if (north==0 && east==0)
	{
		if (flag[1]==0)
		{
		i_min=2;
		i_max=local[0];
		j_min=1;	
		j_max=local[1]-1;
			if (south==0)
			{ 
			i_max=local[0]-1;
			if (flag[0]==1) i_max=local[0]-1-pad_size[0];}	
		}
		if (flag[1]==1)
		{
		i_min=2;
		i_max=local[0];
		j_min=1;	
		j_max=local[1]-1-pad_size[1];
			if (south==0)
			{ 
			i_max=local[0]-1;
			if (flag[0]==1) i_max=local[0]-1-pad_size[0];}	
			
		}
	
	}	
	if (south==0 && east==0)
	{
		if (flag[1]==0 && flag[0]==0)
		{
		i_min=1;
		i_max=local[0]-1;
		j_min=1;	
		j_max=local[1]-1;
		}
		if (flag[0]==1 && flag[1]==0)
		{
		i_min=1;
		i_max=local[0]-1-pad_size[0];
		j_min=1;	
		j_max=local[1]-1;
		
		}
		if (flag[0]==0 && flag[1]==1)
		{
		i_min=1;
		i_max=local[0]-1;
		j_min=1;	
		j_max=local[1]-1-pad_size[1];	
		}
		if (flag[0]==1 && flag[1]==1)
		{
		i_min=1;
		i_max=local[0]-1-pad_size[0];
		j_min=1;	
		j_max=local[1]-1-pad_size[1];
		
		}
	}
	
	if (south==0 && north==0)
	{
	i_min=2;
	i_max=local[0]-1;
	j_min=1;
	j_max=local[1];
	if (flag[0]==1) i_max=local[0]-1-pad_size[0];
	
	}
	
	if (east==0 && west==0) 
	{
	i_min=1;
	i_max=local[0];
	j_min=2;
	j_max=local[1]-1;
	if (flag[1]==1) j_max=local[1]-1-pad_size[1];
	}

	if (north==0 && west==0 &&east==0)
	{
	i_min=2;
	i_max=local[0];
	j_min=2;
	if (flag[1]==0 ){ j_max=local[1]-1;}
	if (flag[1]==1 ){ j_max=local[1]-1-pad_size[1];}
	}
	if (south==0 && west==0 && north==0)
	{
	i_min=2;
	j_min=2;	
	j_max=local[1];
	if (flag[0]==0){i_max=local[0]-1;}
	if (flag[0]==1)	{i_max=local[0]-1-pad_size[0];}
	}

	if (north==0 && east==0 && south==0)
	{
		if (flag[1]==0)
		{
		i_min=2;
		i_max=local[0]-1;
		j_min=1;	
		j_max=local[1]-1;
		if (flag[0]==1) i_max=local[0]-1-pad_size[0];	
		}
		if (flag[1]==1)
		{
		i_min=2;
		i_max=local[0]-1;
		j_min=1;	
		j_max=local[1]-1-pad_size[1];
		if (flag[0]==1) i_max=local[0]-1-pad_size[0];
		}
	}
		
	if (west==0 && east==0 && south==0)
	{
		if (flag[0]==0)
		{
		i_min=1;
		i_max=local[0]-1;
		j_min=2;	
		j_max=local[1]-1;
		if (flag[1]==1) j_max=local[1]-1-pad_size[1];	
		}
		if (flag[0]==1)
		{
		i_min=1;
		i_max=local[0]-1-pad_size[0]; printf("/////imax:%d//////",pad_size[0]);
		j_min=2;	
		j_max=local[1]-1;
		if (flag[1]==1) j_max=local[1]-1-pad_size[1];
		}
	}
	
		

	if (south==0 && west==0 && east==0 && north==0)
	{
	i_min=2;
	i_max=local[0]-1;
	j_min=2;
	j_max=local[1]-1;
	}
  
MPI_Request recv_request_prev[2],send_request_prev[2],recv_request_curr[2],send_request_curr[2];
MPI_Status status[2];
int nmsgs_prev_req,nmsgs_curr_req,nmsgs_curr_send,nmsgs_prev_send;
  
gettimeofday(&tts,NULL);


//MPI_Barrier(MPI_COMM_WORLD);

#ifdef TEST_CONV
for (t=0;t<T && !global_converged;t++) {
	#endif
	#ifndef TEST_CONV
	#undef T
	#define T 256
	for (t=0;t<T;t++) {
	#endif
	gettimeofday(&tcomms,NULL);
	nmsgs_prev_req=0;
	nmsgs_curr_req=0;
	nmsgs_curr_send=0;
	nmsgs_prev_send=0;

	if (east==1)
	{
		
	MPI_Irecv(&(u_previous[0][local[1]+1]),1,column,rank+1,rank+1,MPI_COMM_WORLD,&recv_request_prev[nmsgs_prev_req]) ;
	nmsgs_prev_req++;
	}



	if (west==1)
	{
	
	MPI_Irecv(&(u_current[0][0]),1,column,rank-1,rank-1,MPI_COMM_WORLD,&recv_request_curr[nmsgs_curr_req]) ;
	nmsgs_curr_req++;
	MPI_Isend(&(u_previous[0][1]),1,column,rank-1,rank,MPI_COMM_WORLD,&send_request_prev[nmsgs_prev_send]);
	nmsgs_prev_send++;
	}
 
	if (north==1)
	{
	MPI_Irecv(&(u_current[0][0]),local[1]+2,MPI_DOUBLE,rank-grid[1],rank-grid[1],MPI_COMM_WORLD,&recv_request_curr[nmsgs_curr_req]) ;
	nmsgs_curr_req++;
	MPI_Isend(&(u_previous[1][0]),local[1]+2,MPI_DOUBLE,rank-grid[1],rank,MPI_COMM_WORLD,&send_request_prev[nmsgs_prev_send]);
	nmsgs_prev_send++;
	
	}

	if (south==1)
	{
	MPI_Irecv(&(u_previous[local[0]+1][0]),local[1]+2,MPI_DOUBLE,rank+grid[1],rank+grid[1],MPI_COMM_WORLD,&recv_request_prev[nmsgs_prev_req]) ;
	nmsgs_prev_req++;
	}
	MPI_Waitall(nmsgs_prev_send, &send_request_prev[0], &status[0]);
	MPI_Waitall(nmsgs_prev_req, &recv_request_prev[0], &status[0]);
	MPI_Waitall(nmsgs_curr_req, &recv_request_curr[0], &status[0]);
		
	gettimeofday(&tcommf,NULL);
	
	tcomm+=(tcommf.tv_sec-tcomms.tv_sec)+(tcommf.tv_usec-tcomms.tv_usec)*0.000001;
	
	gettimeofday(&tcs,NULL);

	GaussSeidel(u_previous,u_current,i_min,i_max+1,j_min,j_max+1,omega);
	
	gettimeofday(&tcf,NULL);
	tcomp+=(tcf.tv_sec-tcs.tv_sec)+(tcf.tv_usec-tcs.tv_usec)*0.000001;
	
	gettimeofday(&tcomms,NULL);
	if (east==1)
	{	
	MPI_Isend(&(u_current[0][local[1]]),1,column,rank+1,rank,MPI_COMM_WORLD,&send_request_curr[nmsgs_curr_send]);
	nmsgs_curr_send++;
	}
	
	if (south==1)
	{
	MPI_Isend(&(u_current[local[0]][0]),local[1]+2,MPI_DOUBLE,rank+grid[1],rank,MPI_COMM_WORLD,&send_request_curr[nmsgs_curr_send]);
	nmsgs_curr_send++;
	}
	MPI_Waitall(nmsgs_curr_send, &send_request_curr[0], &status[0]);
	gettimeofday(&tcommf,NULL);
	
	tcomm+=(tcommf.tv_sec-tcomms.tv_sec)+(tcommf.tv_usec-tcomms.tv_usec)*0.000001;

	#ifdef TEST_CONV
        if (t%C==0) {
		
		converged=converge_limits(u_previous,u_current,i_min,i_max+1,j_min,j_max+1);
		MPI_Reduce(&converged,&global_converged,1,MPI_INT,MPI_PROD,0,MPI_COMM_WORLD);
		MPI_Bcast(&global_converged,1,MPI_INT,0,MPI_COMM_WORLD);
	
		}		
	#endif

	swap=u_previous;
	u_previous=u_current;
	u_current=swap;

}

    gettimeofday(&ttf,NULL);

    ttotal=(ttf.tv_sec-tts.tv_sec)+(ttf.tv_usec-tts.tv_usec)*0.000001;

    MPI_Reduce(&ttotal,&total_time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&tcomp,&comp_time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&tcomm,&comm_time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
 


    //----Rank 0 gathers local matrices back to the global matrix----//
   
    if (rank==0) {
            U=allocate2d(global_padded[0],global_padded[1]);    
    }
    MPI_Gatherv(&(u_previous[1][1]),1,local_block,&(U[0][0]),scattercounts,scatteroffset,global_block,0,MPI_COMM_WORLD);
    if (rank==0) {
		printf("------------------------- \n");
		printf("Seidelsor X %d Y %d Px %d Py %d Iter %d ComputationTime %lf CommunicationTime %lf TotalTime %lf midpoint %lf\n",global[0],global[1],grid[0],grid[1],t-1,comp_time,comm_time,total_time,U[global[0]/2][global[1]/2]);
	
		#ifdef PRINT_RESULTS
		char * s=malloc(50*sizeof(char));
	    	sprintf(s,"resSeidelsorMPI_%dx%d_%dx%d",global[0],global[1],grid[0],grid[1]);
    		fprint2d(s,U,global[0],global[1]);
	    	free(s);
		#endif

    }
    MPI_Finalize();
    return 0;
}

