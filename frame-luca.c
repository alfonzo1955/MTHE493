#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

void localConvolve(int imgRows, int imgCols, int filtRows, int filtCols, int** imgIn, int** imgOut, double** filter,int x, int y);
void convolve(int imgRows, int imgCols, int filtRows, int filtCols, int** imgIn, int** imgOut, double** filter);
void localHistogram(int* histogram, int** img, int** img2, int filtSize, int imgRows, int imgCols, int a, int b);
void computeHistogram(int imgRows, int imgCols, int** img, int* histogram);
double energy(int noFilts, float* lambdas, int* histogram);
void gaussian(double** filter, double sigma, int filtRows, int filtCols);
void gabor(double** filter, double sigma, int theta, int filtRows, int filtCols);

int** imgCopy1;

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Invalid number of arguments\n%s [BASENAME] [COLS] [ROWS]\n",argv[0]);
        return 1;
    }
    int i=0, j=0, k=0, l=0;
    char fName[255];
    char oName[255];
    char suffix[255];
    char* basename = argv[1];
    int cols = atoi(argv[2]);
    int rows = atoi(argv[3]);

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

    strcpy(fName,basename);
    strcat(fName,".csv");
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

    int** filteredImg;
    filteredImg = malloc(rows * sizeof(*filteredImg));
    for (i=0;i<rows;i++)
    {
        filteredImg[i] = malloc(cols * sizeof(*filteredImg[i]));
    }

    int** oldfilteredImg;
    oldfilteredImg = malloc(rows * sizeof(*oldfilteredImg));
    for (i=0;i<rows;i++)
    {
        oldfilteredImg[i] = malloc(cols * sizeof(*oldfilteredImg[i]));
    }

    int** synthImg;
    synthImg = malloc(rows * sizeof(*synthImg));
    for (i=0;i<rows;i++)
    {
        synthImg[i] = malloc(cols * sizeof(*synthImg[i]));
    }

    imgCopy1 = malloc(rows * sizeof(*imgCopy1));
    for (i=0;i<rows;i++)
    {
        imgCopy1[i] = malloc(cols * sizeof(*imgCopy1[i]));
    }

    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            filteredImg[i][j] = img[i][j];
    }

    for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            oldfilteredImg[i][j] = img[i][j];
    }
	
	for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            origImg[i][j] = img[i][j];
    }

	for (i=0;i<rows;i++)
    {
        for (j=0;j<cols;j++)
            imgCopy1[i][j] = img[i][j];
    }
////////////////////////////////////////////////////////////////////////////////

    long epsilon = 1000;
    double delta = 0.05;
    long diff;
    double sum;
    double prob;
    float curEnergy;
    float beta;
    int noFilts = 4;

    double probs[256];
    double energies[256];
    double randomNumber;
    double initNbhdEnergy,curNbhdEnergy,current;
    double max;
    int randInt;
    int backup;

    int obsHistogram1[256];
    int obsHistogram2[256];
    int obsHistogram3[256];
    int obsHistogram4[256];
    int synthHistogram1[256];
    int synthHistogram2[256];
    int synthHistogram3[256];
    int synthHistogram4[256];
    

    // filter stuff
    int filtSize = 5;
    int filtRows = filtSize;
    int filtCols = filtSize;
    double** filter1;
    filter1 = malloc(filtRows * sizeof(*filter1));
    for (i=0; i<filtCols; i++)
        filter1[i] = malloc(filtRows * sizeof(*filter1[i]));
    gabor(filter1, 2, 0, filtRows, filtCols);
    double** filter2;
    filter2 = malloc(filtRows * sizeof(*filter2));
    for (i=0; i<filtCols; i++)
        filter2[i] = malloc(filtRows * sizeof(*filter2[i]));
    gabor(filter2, 2, 30, filtRows, filtCols);
    double** filter3;
    filter3 = malloc(filtRows * sizeof(*filter3));
    for (i=0; i<filtCols; i++)
        filter3[i] = malloc(filtRows * sizeof(*filter3[i]));
    gabor(filter3, 2, 60, filtRows, filtCols);
    double** filter4;
    filter4 = malloc(filtRows * sizeof(*filter4));
    for (i=0; i<filtCols; i++)
        filter4[i] = malloc(filtRows * sizeof(*filter4[i]));
    gabor(filter4, 2, 90, filtRows, filtCols);

    // main code
    convolve(rows,cols,filtRows,filtCols,img,filteredImg,filter1);  
    computeHistogram(rows,cols,filteredImg,obsHistogram1);
    convolve(rows,cols,filtRows,filtCols,img,filteredImg,filter2);  
    computeHistogram(rows,cols,filteredImg,obsHistogram2);
    convolve(rows,cols,filtRows,filtCols,img,filteredImg,filter3);  
    computeHistogram(rows,cols,filteredImg,obsHistogram3);
    convolve(rows,cols,filtRows,filtCols,img,filteredImg,filter4);  
    computeHistogram(rows,cols,filteredImg,obsHistogram4);

    float lambdas[noFilts][256];

    for(i=0;i<noFilts;i++){
        for (j=0;j<256;j++)
            lambdas[i][j] = 0;
    }
    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++)
            synthImg[i][j] = rand()%256;
    }
    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++)
            imgCopy1[i][j] = synthImg[i][j];
    }
    int count = 0;
    int loopNo = 0;
    do
    {
        loopNo++;
        convolve(rows,cols,filtRows,filtCols,synthImg,filteredImg,filter1);
        computeHistogram(rows,cols,filteredImg,synthHistogram1);
        convolve(rows,cols,filtRows,filtCols,synthImg,filteredImg,filter2);
        computeHistogram(rows,cols,filteredImg,synthHistogram2);
        convolve(rows,cols,filtRows,filtCols,synthImg,filteredImg,filter3);
        computeHistogram(rows,cols,filteredImg,synthHistogram3);
        convolve(rows,cols,filtRows,filtCols,synthImg,filteredImg,filter4);
        computeHistogram(rows,cols,filteredImg,synthHistogram4);

        for (i=0;i<256;i++)
            lambdas[0][i] += delta * (synthHistogram1[i] - obsHistogram1[i]);
        for (i=0;i<256;i++)
            lambdas[1][i] += delta * (synthHistogram2[i] - obsHistogram2[i]);
        for (i=0;i<256;i++)
            lambdas[1][i] += delta * (synthHistogram3[i] - obsHistogram3[i]);
        for (i=0;i<256;i++)
            lambdas[1][i] += delta * (synthHistogram4[i] - obsHistogram4[i]);

        curEnergy = energy(noFilts,lambdas[0],synthHistogram1)+energy(noFilts,lambdas[1],synthHistogram2)+energy(noFilts,lambdas[2],synthHistogram3)+energy(noFilts,lambdas[3],synthHistogram4);
        prob = exp(-1 * curEnergy);

        count = 0;
        for (k=1;k<=100;k+=1){
            count++;
            beta = 3 / log(k+1);
            curEnergy = energy(noFilts,lambdas[0],synthHistogram1)+energy(noFilts,lambdas[1],synthHistogram2)+energy(noFilts,lambdas[2],synthHistogram3)+energy(noFilts,lambdas[3],synthHistogram4);
            for (i=0;i<rows;i++){
                if ((i+1)%rows == 0)
                    printf("count: %d\n",count);
                for (j=0;j<cols;j++){
                        //metropolis start
                        randomNumber = rand()% 256;
                        randInt = randomNumber;
                        imgCopy1[i][j] = randInt;

                        localConvolve(rows,cols,filtRows,filtCols,synthImg,oldfilteredImg,filter1,i,j);
                        localConvolve(rows,cols,filtRows,filtCols,imgCopy1,filteredImg,filter1,i,j);
                        localHistogram(synthHistogram1, oldfilteredImg, filteredImg, filtSize, rows, cols,i, j);

                        localConvolve(rows,cols,filtRows,filtCols,synthImg,oldfilteredImg,filter2,i,j);
                        localConvolve(rows,cols,filtRows,filtCols,imgCopy1,filteredImg,filter2,i,j);
                        localHistogram(synthHistogram2, oldfilteredImg, filteredImg, filtSize, rows, cols, i, j);

                        localConvolve(rows,cols,filtRows,filtCols,synthImg,oldfilteredImg,filter3,i,j);
                        localConvolve(rows,cols,filtRows,filtCols,imgCopy1,filteredImg,filter3,i,j);
                        localHistogram(synthHistogram3, oldfilteredImg, filteredImg, filtSize, rows, cols, i, j);

                        localConvolve(rows,cols,filtRows,filtCols,synthImg,oldfilteredImg,filter4,i,j);
                        localConvolve(rows,cols,filtRows,filtCols,imgCopy1,filteredImg,filter4,i,j);
                        localHistogram(synthHistogram4, oldfilteredImg, filteredImg, filtSize, rows, cols, i, j);

                        current = energy(noFilts,lambdas[0],synthHistogram1)+energy(noFilts,lambdas[1],synthHistogram2)+energy(noFilts,lambdas[2],synthHistogram3)+energy(noFilts,lambdas[3],synthHistogram4);
                        //
                        //printf("%f \n",curEnergy-current);   
                        //                
                        if (current < curEnergy){
                            curEnergy = current;
                            synthImg[i][j] = randInt;
                        }
                        else{
                            randomNumber = rand()/RAND_MAX;
                            prob = exp(-(1/beta) * (current - curEnergy));
                            if (randomNumber < prob){
                                curEnergy = current;
                                synthImg[i][j] = randInt;
                            }
                            else{
                                continue;
                            }
                        }
                        imgCopy1[i][j]=synthImg[i][j];
                }
            }
        }
        printf("Loop %d done\n",loopNo);
        diff = 0;
        
        /*for (i=0;i<256;i++){
            diff += abs(obsHistogram1[i] - synthHistogram1[i]) + abs(obsHistogram1[i] - synthHistogram1[i]);
        }
        diff = diff/2;
        printf("diff: %d\n",diff);*/
    } while (loopNo < 50);
    //} while (diff > epsilon);*/

    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++){
            img[i][j] = synthImg[i][j];
        }
    }
    
////////////////////////////////////////////////////////////////////////////////

    for (i=0;i<rows;i++)
    {
        free(filteredImg[i]);
    }
    free(filteredImg);

    for (i=0;i<rows;i++)
    {
        free(synthImg[i]);
    }
    free(synthImg);

    for (i=0;i<rows;i++)
    {
        free(imgCopy1[i]);
    }
    free(imgCopy1);

/////////////////////////////////////////////////////////////////////////////////

    strcpy(oName,basename);
    strcat(oName,"-filtered.csv");
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

double energy(int noFilts, float *lambdas, int* histogram){
    int i;
    float ans=0;
        for (i=0;i<256;i++){
            ans += lambdas[i] * histogram[i];
        }
    return ans;
}

void localHistogram(int* histogram, int** img1, int** img2, int filtSize, int imgRows, int imgCols, int a, int b){

    int x,y;
    int filtRows = filtSize;
    int filtCols = filtSize;
    int m = (filtRows-1)/2;
    int n = (filtCols-1)/2;
    
    for(x=a-m; x<a+m; x++){
        for(y=b-n; y<b+n; y++){
            if(x<m || x>=imgRows-m || y<n || y>=imgCols-n)
                continue;
            if(img1[x][y]<0)
                histogram[0]--;
            else if(img1[x][y]>255)
                histogram[255]--;
            else if(histogram[img1[x][y]] != 0)
                histogram[img1[x][y]]--;
            if(img2[x][y]<0)
                histogram[0]++;
            else if(img2[x][y]>255)
                histogram[255]++;
            else
                histogram[img2[x][y]]++;
        }
    } 
}

void computeHistogram(int imgRows, int imgCols, int** img, int* histogram){
    int i,j,k;

    for (i=0;i<256;i++)
        histogram[i] = 0;

    for (i=0;i<imgRows;i++){
        for (j=0;j<imgCols;j++){
            if (img[i][j] < 0)
                histogram[0]++;
            else if (img[i][j] > 255)
                histogram[255]++;
            else
                histogram[img[i][j]]++;
        }
    }
}

void localConvolve(int imgRows, int imgCols, int filtRows, int filtCols, int** imgIn, int** imgOut, double** filter,int a, int b){

    int x,y,i,j;
    double sum;
    int m = (filtRows-1)/2;
    int n = (filtCols-1)/2;

    for(x=a-m; x<a+m; x++){
        for(y=b-n; y<b+n; y++){
            sum = 0;
            if(x<m || x>=imgRows-m || y<n || y>=imgCols-n)
                continue;
            for(i=-m; i<=m; i++){
                for(j=-n; j<=n; j++){
                    sum = sum + filter[j+n][i+m]*imgIn[x-j][y-i];  
                }
            }
        imgOut[x][y] = sum;
        }
    } 
}

void convolve(int imgRows, int imgCols, int filtRows, int filtCols, int** imgIn, int** imgOut, double** filter){

    int x,y,i,j;
    double sum;
    int m = (filtRows-1)/2;
    int n = (filtCols-1)/2;

    for(x=m; x<imgRows-m; x++){
        for(y=n; y<imgCols-m; y++){
            sum = 0;
            for(i=-m; i<=m; i++){
                for(j=-n; j<=n; j++){
                    sum = sum + filter[j+n][i+m]*imgIn[x-j][y-i];  
                }
            }
        imgOut[x][y] = sum;
        }
    } 
}

void gaussian(double** filter, double sigma, int filtRows, int filtCols){

    int i;

    double r;
    double t = 2.0 * sigma * sigma;
    int a = filtRows/2;
    int b = filtCols/2;
    // sum is for normalization
    double sum = 0.0;
    // generate kernel
    for (int x = -a; x <= a; x++){
        for(int y = -b; y <= b; y++){
            r = sqrt(x*x + y*y);
            filter[x + a][y + b] = (exp(-(r*r)/t))/(M_PI * t);
            sum += filter[x + a][y + b];
        }
    }
    // normalize the Kernel
    for(int i = 0; i < filtRows; ++i){
        for(int j = 0; j < filtCols; ++j){
            filter[i][j] /= sum;
        }
    }
}

void gabor(double** filter, double sigma, int theta, int filtRows, int filtCols){

    int x,y;
    double xp,yp,t,s;
    double m = (filtRows)/2;
    double n = (filtCols)/2;

    // radian conversion
    t = theta * M_PI/180;
    s = sigma;

    for(x=0; x<filtRows; x++){
        for(y=0; y<filtCols; y++){
            xp = (x-m)*cos(t) + (y-n)*sin(t);
            yp = -(x-m)*sin(t) + (y-n)*cos(t);
            filter[x][y] = exp(-(1/(4*s*s))*(4*(xp*xp+yp*yp)))*cos(2*M_PI*xp*(1/(sqrt(2)*s)));
        }
    }

    /*double min = filter[0][0];
    double sum = 0;

    for(x=0; x<filtRows; x++){
        for(y=0; y<filtCols; y++){
            if(filter[x][y]< min)
                min = filter[x][y];
        }
    }
    for(x=0 ; x<filtRows; x++){
        for(y=0; y<filtCols; y++){
            filter[x][y] = filter[x][y]-min;
            sum += filter[x][y];
        }
    }
    for(x=0 ; x<filtRows; x++){
        for(y=0; y<filtCols; y++){
            filter[x][y] = filter[x][y]/sum;
        }
    }*/
}
