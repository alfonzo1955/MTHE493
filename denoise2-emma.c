#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

double absf(double x);
double min(double x, double y);
double energy(int rows, int cols, int** img, int** origImg);
double nbhdEnergy(int rows, int cols, int** img, int** origImg, int x, int y);
int sampleProbability(double beta, int rows, int cols, int** img, int** origImg, double* initEnergy, int x, int y);
int testFcn(int rows, int cols, int** imgIn, int** imgOut);

double lambda = 5;
double sigma = 0;
int** imgCopy1;

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Invalid number of arguments\n%s [INFILE] [OUTFILE] [COLS] [ROWS]\n",argv[0]);
        return 1;
    }
    int i=0, j=0, k=0, l=0;
    char* fName = argv[1];
    char* oName = argv[2];
    int cols = atoi(argv[3]);
    int rows = atoi(argv[4]);

    //size_t count;
    char buffer[5000];
    char *line;
    char *record;

    int** img;
    img = malloc(rows * sizeof *img);
    for (i=0;i<rows;i++)
    {
        img[i] = malloc(cols * sizeof *img[i]);
    }

    int** origImg;
    origImg = malloc(rows * sizeof *origImg);
    for (i=0;i<rows;i++)
    {
        origImg[i] = malloc(cols * sizeof *origImg[i]);
    }

    FILE *file;
    file = fopen(fName,"r");

    i = 0;
    while ((line = fgets(buffer,sizeof(buffer),file)) != NULL)
    {
        record = strtok(line,",");
        j = 0;
        while(record != NULL)
        {
            img[i][j++] = atoi(record);
            record = strtok(NULL,",");
        }
        i++;
    }
    fclose(file);

//////////////////////////////////////////////////////////////////////////////////

    // Initialize prng
    srand(time(NULL));
    // Calculate oscilation
    double osc = 0;
    double locOsc = 0;
    double current = 0;
    double delta;

    int** imgCopy;
    imgCopy = malloc(rows * sizeof(*imgCopy));
    for (i=0;i<rows;i++)
    {
        imgCopy[i] = malloc(cols * sizeof(*imgCopy[i]));
    }

    imgCopy1 = malloc(rows * sizeof(*imgCopy1));
    for (i=0;i<rows;i++)
    {
        imgCopy1[i] = malloc(cols * sizeof(*imgCopy1[i]));
    }

    double initEnergy = energy(rows,cols,img,origImg);
    double initNbhdEnergy = 0;
    double curNbhdEnergy = 0;

    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            imgCopy[i][j] = img[i][j];
    }
	
	for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            origImg[i][j] = img[i][j];
    }

    l = 0;

    /*for (i=0; i<rows; i++)
    {
        for (j=0; j<cols; j++)
        {
            initNbhdEnergy = nbhdEnergy(rows,cols,img,origImg,i,j);
            for (k=0;k<256;k++)
            {
                imgCopy[i][j] = k;
                curNbhdEnergy = nbhdEnergy(rows,cols,imgCopy,origImg,i,j);
                current = initEnergy - initNbhdEnergy + curNbhdEnergy;
                locOsc = current > locOsc ? current : locOsc;
            }
            imgCopy[i][j] = img[i][j];
            osc = locOsc > osc ? locOsc : osc;
        }
    }*/

    //delta = osc;
    //printf("Oscillation: %f\n",delta);

    double beta;
    double curEnergy;


    //Main loop
    for (k=1;k<10;k+=1)
    {
        //beta = 1/3 * log(k) * 100;
		//beta = 1/(rows * cols * delta) * log(k);
        beta = 100;
        curEnergy = energy(rows,cols,img,origImg);
        for (i=0;i<rows;i++)
        {
            for (j=0;j<cols;j++)
            {
                img[i][j]=sampleProbability(beta,rows,cols,img,origImg,&curEnergy,j,i);
            }
        }
		
    }
    

//    int count = 0;
//
//    for (i=0;i<rows;i++)
//    {
//        for (j=0;j<cols;j++)
//        {
//            count = count + origImg[i][j];
//        }
//    }
//    printf("%d\n",count);
//
//    count = testFcn(rows,cols,origImg,img);
//    printf("%d\n",count);

    for (i=0;i<rows;i++)
    {
        free(imgCopy[i]);
    }
    free(imgCopy);

    for (i=0;i<rows;i++)
    {
        free(imgCopy1[i]);
    }
    free(imgCopy1);


/////////////////////////////////////////////////////////////////////////////////

    file = fopen(oName,"w");

    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            fprintf(file,"%d,",img[i][j]);
        fprintf(file,"\n");
    }

    fclose(file);

    for (i=0;i<rows;i++)
    {
        free(origImg[i]);
    }
    free(origImg);

    fclose(file);
    for (i=0;i<rows;i++)
    {
        free(img[i]);
    }
    free(img);
    return 0;
}

int testFcn(int rows, int cols, int** imgIn, int** imgOut) //Average
{
    int count = 0;
    int i,j;

    for (i=1;i<rows-1;i++)
    {
        for (j=1;j<cols-1;j++)
        {
            count = count + imgIn[i][j];
            imgOut[i][j] = (imgIn[i][j] + imgIn[(i+1)][j] + imgIn[(i-1)][j] + imgIn[i][(j+1)] + imgIn[i][(j-1)])/5;
        }
    }
    return count;
}

double energy(int rows, int cols, int** img, int** origImg)
{
    int i,j,k;


    double ans = 0;

    double pt1 = 0;
    double pt2 = 0;

    double diff = 0;

    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
        {
            diff = origImg[i][j] - img[i][j];
            pt1 += (abs(diff))*(abs(diff))/255;
        }
    }
    
    pt1 = pt1 * 1/(2.0 * sigma*sigma);
    
    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
        {
            if (j+1 < cols)
            {
                diff = img[i][j] - img[i][j+1];
                diff = diff/255;
                pt2 += diff * diff;
            }
            if (j-1 >=  0)
            {
                diff = img[i][j] - img[i][j-1];
                diff = diff/255;
                pt2 += diff * diff;
            }
            if (i+1 < rows)
            {
                diff = img[i][j] - img[i+1][j];
                diff = diff/255;
                pt2 += diff * diff;
            }
            if (i-1 >= 0)
            {
                diff = img[i][j] - img[i-1][j];
                diff = diff/255;
                pt2 += diff * diff;
            }
        }
    }

    pt2 = pt2 * lambda;

    ans = pt1 + pt2;
    return ans;
}

double nbhdEnergy(int rows, int cols, int** img, int** origImg, int x, int y)
{
    int i,j;
    int xCoord, yCoord;

    double pt1 = 0;
    double pt2 = 0;
    double ans = 0;

    double diff = 0;


    for (i=-1;i<=1;i++)
    {
        for (j=-1;j<=1;j++)
        {
            yCoord = y+i;
            xCoord = x+j;

            if (xCoord < cols && xCoord >= 0 && yCoord < rows && yCoord >= 0)
            {
                pt1 += abs(origImg[yCoord][xCoord] - img[yCoord][xCoord])*abs(origImg[yCoord][xCoord] - img[yCoord][xCoord])/255.0;
				
            }
        }
    }
    pt1 = pt1 * 1/(2.0 * sigma*sigma);

    for (i=-1;i<=1;i++)
    {
        for (j=-1;j<=1;j++)
        {
            yCoord = y+i;
            xCoord = x+j;

            if (xCoord < cols && xCoord >= 0 && yCoord < rows && yCoord >= 0)
            {
				if (xCoord+1 < cols)
                {
                    diff += img[yCoord][xCoord] - img[yCoord][xCoord+1];
                    pt2 += diff * diff/255;
                }
                if (xCoord-1 >= 0)
                {
                    diff += img[yCoord][xCoord] - img[yCoord][xCoord-1];
                    pt2 += diff * diff/255;
                }
                if (yCoord+1 < rows)
                {
                    diff += img[yCoord][xCoord] - img[yCoord+1][xCoord];
                    pt2 += diff * diff/255;
                }
                if (yCoord-1 >= 0)
                {
                    diff += img[yCoord][xCoord] - img[yCoord-1][xCoord];
                    pt2 += diff * diff/255;
                }
            }
        }
    }

    pt2 = pt2 * lambda;
    ans = pt1 + pt2;
    return ans;
}

int sampleProbability(double beta, int rows, int cols, int** img, int** origImg, double* initEnergy, int x, int y)
{
    double probs[256];
    double energies[256];
    double sum = 0;
    double randomNumber;
    double initNbhdEnergy,curNbhdEnergy,current;

    int i,j,k;


    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            imgCopy1[i][j] = img[i][j];
    }


    initNbhdEnergy = nbhdEnergy(rows,cols,img,origImg,x,y);
    for (k=0;k<256;k++)
    {
        imgCopy1[y][x] = k;
        curNbhdEnergy = nbhdEnergy(rows,cols,imgCopy1,origImg,x,y);
        current = *initEnergy - initNbhdEnergy + curNbhdEnergy; 
        energies[k] = current;
        probs[k] = exp(-1 * beta * current);
//        printf("%f %f %f\n",beta,current,probs[k]);
        sum += probs[k];
    }

    for (k=0;k<256;k++)
    {
        probs[k] = probs[k] / sum;
    }

    for (k=1;k<256;k++)
    {
//        printf("%f\n",probs[k]);
        probs[k] = probs[k-1] + probs[k]; //PDF to CDF
    }
    
    randomNumber = rand();
    randomNumber = randomNumber/RAND_MAX;

    k = 0;
    while (1)
    {
        if (randomNumber < probs[k])
            break;
        k++;
    }

    *initEnergy = energies[k];
    return k;
}

double absf(double x)
{
    return x >= 0 ? x : -1 * x;
}

double min(double x, double y)
{
    return x > y ? x : y;
}
