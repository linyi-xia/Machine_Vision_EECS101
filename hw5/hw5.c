#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "StdAfx.h"

#define ROWS   		 		 480
#define COLS				 640
#define LOGICAL_X_MIN			-4.0
#define LOGICAL_X_MAX			 4.0
#define LOGICAL_Y_MIN			-4.0
#define LOGICAL_Y_MAX			 4.0

void clear( unsigned char image[][COLS] );
int plot_logical_point( float x, float y, unsigned char image[][COLS] );
int plot_physical_point( int x, int y, unsigned char image[][COLS] );
int in_range( int x, int y );
void header( int row, int col, unsigned char head[32] );
void readFile (unsigned char image[][COLS],FILE *fp,char *ifile, char **argv);
void writeFile (unsigned char image[][COLS],FILE *fp, char *ofile, char **argv);
void dedy(int *a, int *b, unsigned char image[][COLS]);
void dedx(int *a, int *b, unsigned char image[][COLS]);

int main( int argc, char **argv )
{
	int				i, j, th, x, y, line1, line2,line3,theta1,theta2,theta3,theta4,rho1,rho2,rho3,rho4,line4;
	double			Ax,Ay,Ac, temp,tempX,tempY,angle1,angle2,angle3,r1,r2,r3,step,rho,theta;
	FILE			*fp;
	unsigned char	image[ROWS][COLS];
	unsigned char	SGM[ROWS][COLS];
	unsigned char	vote[ROWS][COLS];
	char *ifile, *ofilex, *ofiley, *ofileC ,*ofileD,*ofileE;
	unsigned char head[32];
	int max, min, a, b;

	th = 190;
	fp = NULL;

	/* Example to show how to do format conversion */
	/* Input image file */
	ifile = "image.raw";
	/* Output image file */	
	ofilex = "image1x.ras";
	/* Output image file */	
	ofiley = "image1y.ras";
	/* Output image file */	
	ofileC = "image2.ras";
	/* Output image file */	
	ofileD = "image3.ras";
	/* Output image file */	
	ofileE = "image4.ras";

	/* Clear image buffer */
	clear(image);

	/* Read in a raw image */
	readFile (image,fp,ifile, argv);

	/* Create a header */
	header(ROWS, COLS, head);

	//Initialize target array
	clear(SGM);
	clear(vote);

	//---------------------------------------------------y direction de/dy
	max = 1;
	min = 100;
	dedy(&max,&min,image);
	Ay = 255./max;
	for(i=0; i<ROWS; i++)
	{
		for(j=0; j< COLS;j++)
		{
			temp=abs(image[i-1][j-1]+2*image[i-1][j]+image[i-1][j+1]-(image[i+1][j-1] + 2*image[i+1][j]+image[i+1][j+1]));
			SGM[i][j] = temp*Ay;
		}
	}
	/* Save it into a ras image */
	writeFile (SGM,fp, ofiley, argv, head);

	//x direction de/dx
	max = 1;
	min = 100;
	dedx(&max,&min,image);
	Ax = 255./max;
	for(i=0; i<ROWS; i++)
	{
		for(j=0; j< COLS;j++)
		{
			temp = abs((image[i-1][j+1]+2*image[i][j+1]+image[i+1][j+1])-(image[i-1][j-1]+2*image[i][j-1]+image[i+1][j-1]));
			SGM[i][j]= temp*Ax;
		}
	}
	/* Save it into a ras image */
	writeFile (SGM,fp, ofilex, argv, head);

	//----------------------------------------------Compute SGM
	max = 1;
	min = 100;
	for ( i=1;i<ROWS-1;i++ )	//		finding normalizing factor
	{
		for (j=1;j<COLS-1;j++)
		{
			tempX = abs((image[i-1][j+1]+2*image[i][j+1]+image[i+1][j+1])-(image[i-1][j-1]+2*image[i][j-1]+image[i+1][j-1]));
			tempY = abs(image[i-1][j-1]+2*image[i-1][j]+image[i-1][j+1]-(image[i+1][j-1] + 2*image[i+1][j]+image[i+1][j+1]));
			temp = pow(tempX,2)+pow(tempY,2);
			temp = sqrt(temp);

			if (temp > max)
			{
				max = temp;
			}
			if (temp < min)
			{
				min = temp;
			}
		}
	}
	Ac = 255./max;
	//---------------------------------------------------construct SGM target array
	for(i=0; i<ROWS; i++)
	{
		for(j=0; j< COLS;j++)
		{
			tempX = abs((image[i-1][j+1]+2*image[i][j+1]+image[i+1][j+1])-(image[i-1][j-1]+2*image[i][j-1]+image[i+1][j-1]));
			tempY = abs(image[i-1][j-1]+2*image[i-1][j]+image[i-1][j+1]-(image[i+1][j-1] + 2*image[i+1][j]+image[i+1][j+1]));
			temp = pow(tempX,2)+pow(tempY,2);
			temp = sqrt(temp);
			SGM[i][j]=temp*Ac;
		}
	}

	/* Save it into a ras image */
	writeFile (SGM,fp, ofileC, argv, head);


	for(i=0; i<ROWS; i++)
	{
		for(j=0; j< COLS;j++)
		{
			if (SGM[i][j] < th)
			{
				SGM[i][j] = 255;
			}
			else
				SGM[i][j] = 0;
				
		}
	}
	writeFile (SGM,fp, ofileD, argv, head);

	//--------------------------------------------------- voting process
	min = 100;
	max = 1;
	theta = 0;
	for(i=1; i<ROWS-1; i++)
	{
		for(j=1; j< COLS-1;j++)
		{
			if (SGM[i][j] <1)
			{
				for(x = 0; x < ROWS; x++)
				{
					rho = j*cos(-3.14/ROWS*x)+i*sin(-3.14/ROWS*x);
					if( rho > max)
					{
						max = rho;
					}
					else if (rho < min)
					{
						min = rho;
					}
				}
			}
				
		}
	}
	//nomalize rho array by defining step size
	step = (max-min)/COLS;
	//voting begins
	for(i=1; i<ROWS-1; i++)
	{
		for(j=1; j< COLS-1;j++)
		{
			if (SGM[i][j] <1)
			{
				for(x = 1; x < ROWS-1; x++)
				{
					theta=3.14/ROWS*x;
					rho = j*cos(theta)+i*sin(theta);
					y = (rho-min)/step;
					vote[x][y]++;
				}
			}
				
		}
	}

	//--------------------------------------------------- finding theta and rho
	//initialize variables
	line1 = line2 = line3 = line4 = 0;
	theta1 = theta2 = theta3 = theta4 =0;
	rho1 = rho2 = rho3 = 0;
	for(i=2;i<ROWS-2;i++)
	{
		for(j=2;j<COLS-2;j++)
		{
			//finding popular votes
			if(vote[i][j]>line1)
			{
				line4 = line3;
				line3 = line2;
				line2 = line1;
				line1 = vote[i][j];

				theta4 = theta3;
				theta3 = theta2;
				theta2 = theta1;
				theta1 = i;

				rho4 = rho3;
				rho3 = rho2;
				rho2 = rho1;
				rho1 = j;

			}

			//push the stored value forward if they appears to be the same line
			if(theta1 - theta2 < 10)
			{
				theta1 = theta2;
				theta2 = theta3;
				theta3 = theta4;
					
				rho1 = rho2;
				rho2 = rho3;
				rho3 = rho4;

			}
			
		}
	}

	angle1 = 3.14/ROWS * theta1;
	angle2 = 3.14/ROWS * theta2;
	angle3 = 3.14/ROWS * theta3;
	r1 = step*rho1+min;
	r2 = step*rho2+min;
	r3 = step*rho3+min;
	printf("line1,2,3 \t%i, %i, %i, %i \n",line1, line2, line3,line4);
	printf("indexes %i,%i,\t,%i,%i,\t,%i,%i,\t,%i,%i \n",theta1,rho1,theta2,rho2,theta3,rho3,theta4,rho4);
	printf("theta %f, rho %f\n%f, %f\n%f, %f\n",angle1/3.14*180,r1,angle2/3.14*180,r2,angle3/3.14*180,r3);
 	printf("min, max (%i,%i)",min,max);


	//--------------------------------------------------- reconstruct image
	clear(image);
	for(j = 1; j <COLS ; j++ )
	{
		//line1
		i = (r1-j*cos(angle1))/sin(angle1);
		if (i<ROWS && i>0)
		{
			image[i][j] = 255;
		}

		//line2
		i = (r2-j*cos(angle2))/sin(angle2);
		//j = (- cos(angle2)/sin(angle2))*i+r2/sin(angle2);
		if (i<ROWS && i>0)
		image[i][j] = 255;

		//line3
		i = (r3-j*cos(angle3))/sin(angle3);
		//j = (- cos(angle3)/sin(angle3))*i+r3/sin(angle3);
		if (i<ROWS && i>0)
		image[i][j] = 255;
	}
		
	//save reconstructed array
	writeFile (image,fp, ofileE, argv, head);

	return 0;
}

void dedx(int *a, int *b, unsigned char image[][COLS])
{
	int i,temp,j;
	for ( i=1;i<ROWS-1;i++ )	//		de/dx
	{
		for (j=1;j<COLS-1;j++)
		{
			//		de/dx
			temp = abs((image[i-1][j+1]+2*image[i][j+1]+image[i+1][j+1])-(image[i-1][j-1]+2*image[i][j-1]+image[i+1][j-1]));


			if (temp > *a)
			{
				*a = temp;
			}
			if (temp < *b)
			{
				*b = temp;
			}
		}
	}

	return;

}

void dedy(int *a, int *b, unsigned char image[][COLS])
{
	int i,temp,j;
	for ( i=1;i<ROWS-1;i++ )		//		de/dy
	{
		for (j=1;j<COLS-1;j++)
		{
			//			de/dy
			temp=abs(image[i-1][j-1]+2*image[i-1][j]+image[i-1][j+1]-(image[i+1][j-1] + 2*image[i+1][j]+image[i+1][j+1]));

			if (temp > *a)
			{
				*a = temp;
			}
			if (temp < *b)
			{
				*b = temp;
			}
		}
	}

	return;

}

void writeFile (unsigned char SGM[][COLS],FILE *fp, char *ofile, char **argv, unsigned char head[32])
{
	int i;
	/* Save it into a ras image */
	/* Open the file */
	if (!( fp = fopen( ofile, "wb" )))
	fprintf( stderr, "error: could not open %s\n", argv[1] ), exit(1);

	/* Write the header */
	fwrite( head, 4, 8, fp );
	 
	/* Write the image */
	for ( i = 0; i < ROWS; i++ ) 
		fwrite( SGM[i], 1, COLS, fp );

	/* Close the file */
	fclose( fp );
	return;
}
void readFile (unsigned char image[][COLS],FILE *fp,char *ifile, char **argv)
{
	int i = 0;
	/* Open the file */
	if (( fp = fopen( ifile, "rb" )) == NULL )
	{
		fprintf( stderr, "error: couldn't open %s\n", argv[1] );
		exit( 1 );
	}			

	/* Read the file */
	for (i = 0; i < ROWS ; i++ )
		if ( fread( image[i], 1, COLS, fp ) != COLS )
		{
			fprintf( stderr, "error: couldn't read enough stuff\n" );
			exit( 1 );
		}

	/* Close the file */
	fclose( fp );
	return;
}
void clear( unsigned char image[][COLS] )
{
	int	i,j;
	for ( i = 0 ; i < ROWS ; i++ )
		for ( j = 0 ; j < COLS ; j++ ) image[i][j] = 0;
}

int plot_logical_point( float x, float y, unsigned char image[][COLS] )
{
	int	nx, ny;
	float	xc, yc;
	xc = COLS / ((float)LOGICAL_X_MAX - LOGICAL_X_MIN);
	yc = ROWS / ((float)LOGICAL_Y_MAX - LOGICAL_Y_MIN);
	nx = (x - LOGICAL_X_MIN) * xc;
	ny = (y - LOGICAL_Y_MIN) * yc;
	return plot_physical_point( nx, ny, image );
}

int plot_physical_point( int x, int y, unsigned char image[][COLS] )
{
	if (! in_range(x,y) ) return 0;
	return image[y][x] = 255;
}
int in_range( int x, int y )
	{
		return x >= 0 && x < COLS && y >= 0 && y < ROWS;
	}

	void header( int row, int col, unsigned char head[32] )
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */
	
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch ++; 
	head[6] = *ch;
	ch ++;
	head[5] = *ch;
	ch ++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch ++; 
	head[10] = *ch;
	ch ++;
	head[9] = *ch;
	ch ++;
	head[8] = *ch;
	
	ch = (char*)&num;
	head[19] = *ch;
	ch ++; 
	head[18] = *ch;
	ch ++;
	head[17] = *ch;
	ch ++;
	head[16] = *ch;
	
/*
	// Big-endian for unix
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
*/
}