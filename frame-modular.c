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

void testFcn(int** filter,int filtRows,int filtCols)
{
    int i,j;
    for (i=0;i<filtRows;i++)
    {
        for (j=0;j<filtCols;j++)
        {
            printf("%0.3f ",filter[i][j]);
        }
        printf("\n");
    }
}
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Invalid number of arguments\n%s [BASENAME] [COLS] [ROWS]\n",argv[0]);
        return 1;
    }
    int i=0, j=0, k=0, l=0, m=0;
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
    int** imgCopy1;
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

    int min = 999999999;
    double delta = 0.001;
    int diff;
    double sum;
    double prob;
    float curEnergy;
    float beta;
    int noFilts = 6;

    double probs[256];
    double energies[256];
    double randomNumber;
    double current;
    double max;
    int randInt;
    int backup;
    int count = 0;
    int loopNo = 0;

    int** obsHistogram;
    obsHistogram = malloc(noFilts * sizeof(*obsHistogram));
    for (i=0; i<noFilts; i++)
        obsHistogram[i] = malloc(256 * sizeof(*obsHistogram[i]));

    int** synthHistogram;
    synthHistogram = malloc(noFilts * sizeof(*synthHistogram));
    for (i=0; i<noFilts; i++)
        synthHistogram[i] = malloc(256 * sizeof(*synthHistogram[i]));

    int** CsynthHistogram;
    CsynthHistogram = malloc(noFilts * sizeof(*CsynthHistogram));
    for (i=0; i<noFilts; i++)
        CsynthHistogram[i] = malloc(256 * sizeof(*CsynthHistogram[i]));

    // filter stuff
    int filtSize = 5;
    int filtRows[noFilts];
    int filtCols[noFilts];

    filtRows[0] = 1;
    filtRows[1] = 5;
    filtRows[2] = 5;
    filtRows[3] = 5;
    filtRows[4] = 5;
    filtRows[5] = 5;

    for (i=0;i<noFilts;i++)
    {
        filtCols[i] = filtRows[i];
    }

    double*** filter;
    filter = malloc(noFilts * sizeof(double**));
    for (i=0;i<noFilts;i++)
    {
        filter[i] = malloc(filtRows[i] * sizeof(double*));
        for (j=0;j<filtRows[i];j++)
        {
            filter[i][j] = malloc(filtCols[i] * sizeof(double));
        }
    }

    filter[0][0][0] = 1;
    gabor(filter[1],2,0,filtRows[1],filtCols[1]);
    gabor(filter[2],2,90,filtRows[2],filtCols[2]);
    gabor(filter[3],2,45,filtRows[3],filtCols[3]);
    gabor(filter[4],2,135,filtRows[4],filtCols[4]);
    gaussian(filter[5],2,filtRows[5],filtCols[5]);
    //gaussian(filter[3],2,filtRows[3],filtCols[3]);

    // lambda stuff
    float** lambdas;
    lambdas = malloc(noFilts *sizeof(*lambdas));
    for(i=0;i<noFilts;i++)
        lambdas[i] = malloc(256*sizeof(*lambdas[i]));

    // main code
    computeHistogram(rows,cols,filteredImg,obsHistogram[0]);
    for (i=1;i<noFilts;i++)
    {
        convolve(rows,cols,filtRows[i],filtCols[i],img,filteredImg,filter[i]);
        computeHistogram(rows,cols,filteredImg,obsHistogram[i]);
    }

    for(i=0;i<noFilts;i++){
        for (j=0;j<256;j++)
            lambdas[i][j] = 0;
    }
    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++){
            synthImg[i][j] = rand()%256;
        }
    }
    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++)
            imgCopy1[i][j] = synthImg[i][j];
    }
    
    computeHistogram(rows,cols,synthImg,synthHistogram[0]);
    for (i=1;i<noFilts;i++)
    {
        convolve(rows,cols,filtRows[i],filtCols[i],synthImg,filteredImg,filter[i]);
        computeHistogram(rows,cols,filteredImg,synthHistogram[i]);
    }

    do{
        loopNo++;
        for (m=0;m<noFilts;m++)
        {
            for (i=0;i<256;i++){
                lambdas[m][i] = lambdas[m][i] + delta * (synthHistogram[m][i] - obsHistogram[m][i]);
            }
        }
        
        curEnergy = 0;
        for (m=0;m<noFilts;m++)
        {
            curEnergy += energy(noFilts,lambdas[m],synthHistogram[m]);
        }

        prob = exp(-1 * curEnergy);

        count = 0;
        for (k=1;k<=500;k+=1){
            count++;
            beta = 3 / log(k+1);

            curEnergy = 0;
            for (m=0;m<noFilts;m++)
            {
                curEnergy += energy(noFilts,lambdas[m],synthHistogram[m]);
            }
            
            for (i=0;i<rows;i++){
                for (j=0;j<cols;j++){
                        randInt = rand()%256;
                        imgCopy1[i][j] = randInt;

                        for (m=0;m<noFilts;m++)
                        {
                            for(l=0;l<256;l++){
                                CsynthHistogram[m][l] = synthHistogram[m][l];
                            }
                        }

                        localHistogram(synthHistogram[0], synthImg, imgCopy1, filtSize, rows, cols, i, j);
                        for (m=1;m<noFilts;m++)
                        {
                            localConvolve(rows,cols,filtRows[m],filtCols[m],synthImg,oldfilteredImg,filter[m],i,j);
                            localConvolve(rows,cols,filtRows[m],filtCols[m],imgCopy1,filteredImg,filter[m],i,j);
                            localHistogram(synthHistogram[m], oldfilteredImg, filteredImg, filtRows[m], rows, cols, i, j);
                        }

                        current = 0;
                        for (m=0;m<noFilts;m++)
                        {
                            current += energy(noFilts,lambdas[m],synthHistogram[m]);
                        }
            
                        if (current < curEnergy){
                            curEnergy = current;
                            synthImg[i][j] = randInt;
                        }
                        else{
                            randomNumber = rand();
                            randomNumber = randomNumber/RAND_MAX;
                            prob = exp(-(1/beta) * (current - curEnergy));
                            if (randomNumber < prob){
                                curEnergy = current;
                                synthImg[i][j] = randInt;
                            }
                            else{
                                for (m=0;m<noFilts;m++)
                                {
                                    for(l=0;l<256;l++){
                                        synthHistogram[m][l] = CsynthHistogram[m][l];
                                    }
                                }
                                imgCopy1[i][j]=synthImg[i][j];                           
                            }
                        }     
                }
            }
        }
        diff = 0;
        
        if(loopNo%1==0){
            for (m=0;m<noFilts;m++)
            {
                for (i=0;i<256;i++){
                    diff += abs(obsHistogram[m][i] - synthHistogram[m][i]);
                }
            }
        }

        if(loopNo < 1000 && loopNo > 50){
            if(diff<min)
                min = diff;
        }

        printf("\n diff = %d and min = %d\n",diff, min);
        printf("------Loop %d -----\n",loopNo);

    } while ((diff > min || loopNo<=1000) && loopNo<1200);

    for (i=0;i<rows;i++){
        for (j=0;j<cols;j++){
            //img[i][j] = filteredImg[i][j];
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
            ans += lambdas[i]*histogram[i];
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
