//#include <stdio.h>
//
//int main(void)
//{
//    FILE* fphzk = NULL;
//    int i, j, k, offset;
//    int flag;
//    unsigned char buffer[32];
//    unsigned char word[4] = "我";
//    unsigned char key[8] = {
//            0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
//    };
//
//    fphzk = fopen("/home/sensetime/ClionProjects/Test/HZK16", "rb");
//    if(fphzk == NULL){
//        fprintf(stderr, "error hzk16\n");
//        return 1;
//    }
//    offset = (94*(unsigned int)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
//    fseek(fphzk, offset, SEEK_SET);
//    fread(buffer, 1, 32, fphzk);
//    for(k=0; k<32; k++){
//        printf("%02X ", buffer[k]);
//    }
//    for(k=0; k<16; k++){
//        for(j=0; j<2; j++){
//            for(i=0; i<8; i++){
//                flag = buffer[k*2+j]&key[i];
//                printf("%s", flag?"●":"○");
//            }
//        }
//        printf("\n");
//    }
//    fclose(fphzk);
//    fphzk = NULL;
//    return 0;
//}

#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sstream>
#include <string>
#include <fstream>

#define OUTLEN 128

using namespace cv;
using namespace std;

int display16(char *incode, int len);
int display48(char *incode, int len);
int saveImage(Mat im, int x1, int y1, int x2, int y2);
void getFiles(const char * dir_name, vector<char *>& files);
int display16(char *incode, int len, int line);
int display16(char *incode, int len, int colBase, int col1, int col2);
int display16(char *incode, int len, int line, int colBase, int col1, int col2);

int leftX = 10, leftY = 10, maxX = 0, maxY = 0;
Mat image;
int startX = 0, startY = 0;
static int image_count = 0;

int source_id = 1;


int main()
{
    vector<char *> images_vector;

    string inputDir;

    cout << "Input the directory!" << endl;

    cin >> inputDir;

    ofstream outText;
    outText.open("/home/sensetime/Desktop/tttt.txt");

    getFiles(inputDir.c_str(), images_vector);
    int count = 0;

    for (int i = 0; i < images_vector.size(); i++) {

        ifstream infile("/home/sensetime/ClionProjects/Test/medicine_result.txt");

        string line;
        while (getline(infile, line) && count < 50) {

            count++;

            image = imread(images_vector[i], CV_LOAD_IMAGE_COLOR);


            int w = image.cols;
            int h = image.rows;

            cout << w << ", " << h << endl;

            if(! image.data )                              // Check for invalid input
            {
                cout <<  "Could not open or find the image" << std::endl;
                return -1;
            }

            cout << line << endl;
            cout << line.size() / 3 << endl;

            char* string = new char[line.size() + 1];
            copy(line.begin(), line.end(), string);
            string[line.size()] = '\0';


            iconv_t cd;
            size_t inbuf_len = strlen(string);
            char outbuf[OUTLEN];
            char *pin = string;
            char *pout = &outbuf[0];  //用"pout=&outbuf" 会引发SIGSERV信号，导致段错误
            size_t outbuf_len = OUTLEN;

            memset(outbuf, 0, OUTLEN); //清空输出缓存，否则会有意外结果的
            printf("Originial Data:\n"); //打印转换前的一些参数和信息，以进行比较
            printf("\tpin str: %s, outbuf str:%s\n", pin, outbuf);
            printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
            printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

            cd = iconv_open("GB2312", "UTF8");
            if(cd == 0)
                return EXIT_FAILURE;
            int count = iconv(cd, &pin, &inbuf_len, &pout, &outbuf_len);
            printf("iconv count : %d\n", count);//观察iconv返回值，理解不可逆转换含义
            iconv_close(cd);

            printf("After Converted Data:\n"); //注意发生变化的变量
            printf("\tpin str: %s, gb2312 str:%s\n", pin, outbuf );
            printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
            printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

            int i,j;

//            if (h < 100) {
//                leftY = 10;
//            } else {
//                leftY = (int)(rand() % 60);
//            }
//
//            if ( w < 100) {
//                leftX = 10;
//            } else {
//                leftX = (int)(rand() % 50);
//            }


//            leftY = (int)(rand() % (h - 60));
//            leftX = (int)(rand() % 50);

            leftX = 10;
            leftY = 10;

            startX = leftX;
            startY = leftY;

            int maxChar = (w - startX) / 48;

            // 横纹隔断
            int horizontal_break = (int) (rand() % 50);
            // 竖向扭曲
            int colBase = (int) (rand() % 6);
            int col1 = -1, col2 = -1;

            if (colBase >= 0 && colBase < 2) {
                col1 = (int) (rand() % 15);
                col2 += 1;
            }

            for(i = 0; i < strlen(outbuf) && i < (maxChar - 1) * 2; i += 2)
            {
                display16 (outbuf+i, 2, horizontal_break, colBase, col1, col2); //使用HZK16字库显示GB2312编码的中文点阵
            }

            startX -= 4;
            startY -= 4;

            leftY = leftY + 3 * 2 * 8 - 3;
            leftX = leftX - 4;

            saveImage(image, startX, startY, leftX, leftY);

            outText << image_count - 1 << ".jpg" << " " << line.substr(0, (maxChar - 1) * 3) << endl;

            delete[] string;
        }
    }

//    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
//    imshow( "Display window", image );                   // Show our image inside it.



//    waitKey(0);

    return EXIT_SUCCESS;
}

void getFiles(const char * dir_name, vector<char *>& files) {
    // check the parameter !
    if( NULL == dir_name )
    {
        cout<<" dir_name is null ! "<<endl;
        return;
    }

    // check if dir_name is a valid dir
    struct stat s;
    lstat( dir_name , &s );
    if( ! S_ISDIR( s.st_mode ) )
    {
        cout<<"dir_name is not a valid directory !"<<endl;
        return;
    }

    struct dirent * filename;    // return value for readdir()
    DIR * dir;                   // return value for opendir()
    dir = opendir( dir_name );
    if( NULL == dir )
    {
        cout<<"Can not open dir "<<dir_name<<endl;
        return;
    }
    cout<<"Successfully opened the dir !"<<endl;

    /* read all the files in the dir ~ */
    while( ( filename = readdir(dir) ) != NULL )
    {
        // get rid of "." and ".."
        if( strcmp( filename->d_name , "." ) == 0 ||
            strcmp( filename->d_name , "..") == 0    )
            continue;

        //	if (filename ->d_type == DT_DIR) {
        //		char * fname = filename ->d_name;
//
//			char *s = new char[strlen(dir_name)+strlen(fname)+20];
        //	strcpy(s, "\"");
        //	strcat(s,dir_name);
//			strcpy(s,dir_name);
//			strcat(s,fname);
//			strcat(s, "/");
//			//strcat(s, "\"");
//			getFiles(s, files);
//		} else {
        cout << filename->d_name << endl;
        char *fname = filename->d_name;

        char *s = new char[strlen(dir_name) + strlen(fname) + 1];
        strcpy(s, dir_name);
        strcat(s, fname);

        files.push_back(s);
//		}
    }
}

string type2str(int type) {
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

bool randomBool() {
    return rand() % 2 == 1;
}

int display16(char *incode, int len, int line, int colBase, int col1, int col2) {
    int i, j, k;
    char mat[16][2];
    FILE *HZK = NULL;
    unsigned char qh, wh;
    unsigned long offset;

    qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
    wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
    offset = (94 * (qh - 1) + (wh - 1)) * 32; //得到汉字在HZK16字库中的字节偏移位置
    printf("区码：%d,位码：%d\n", qh, wh);

    if ((HZK = fopen("/home/sensetime/ClionProjects/Test/HZK16", "rb")) == NULL) {
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }
    fseek(HZK, offset, SEEK_SET);
    fread(mat, 32, 1, HZK);//读取汉字的16*16点阵字模
    fclose(HZK);

    cout << type2str(image.type()) << endl;

    CvScalar color = CV_RGB(105,105,105);

    int baisX = 0, baisY = 0, bais1, bais2 = 0;

    bool bigger = randomBool();

    if (bigger) {
        bais1 = 2;
    } else
        bais1 = -2;

    bigger = randomBool();

    if (bigger) {
        bais2 = 2;
    } else
        bais2 = -2;

    for (i = 0; i < 16; i++)  //显示点阵
    {
        if (line >= 0 && line < 16) {
            if (i == line) {
                continue;
            }
        }

        for (j = 0; j < 2; j++)
            for (k = 0; k < 8; k++) {
                if (mat[i][j] & (0x80 >> k)) {
                    printf("*");
                    if (colBase >= 0 && colBase < 2 && col1 >= 0 && col1 < 8 && col2 >= 0 && col2 < 8) {
                        if (j == colBase && (k == col1 || k == col2)) {
                            baisX = bais1;
                            baisY = bais2;
                        } else {
                            baisX = 0;
                            baisY = 0;
                        }
                    } else {
                        baisX = 0;
                        baisY = 0;
                    }
                    Point P1(leftX + 3 * (j * 8 + k) + 1 + baisX , leftY + 3 * i + 1 + baisY);
                    circle(image, P1, 1, color, -1);

//                    Point P1((leftX + j * 8 + k) + 1, leftY + i + 1);
//                    circle(image, P1, 0, color);
//
//                    Point P1(leftX + 2 * (j * 8 + k) + 1, leftY + 2 * i + 1);
//                    circle(image, P1, 2, color, -1);
                }
                else
                    printf(" ");
            }
        printf("\n");
    }

    leftX = leftX + 3 * 2 * 8 + 1;

//    leftX = leftX + 2 * 8 + 1;

//    leftX = leftX + 2 * 2 * 8 + 1;

    return EXIT_SUCCESS;
}


int display16(char *incode, int len) {
    int i, j, k;
    char mat[16][2];
    FILE *HZK = NULL;
    unsigned char qh, wh;
    unsigned long offset;

    qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
    wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
    offset = (94 * (qh - 1) + (wh - 1)) * 32; //得到汉字在HZK16字库中的字节偏移位置
    printf("区码：%d,位码：%d\n", qh, wh);

    if ((HZK = fopen("/home/sensetime/ClionProjects/Test/HZK16", "rb")) == NULL) {
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }
    fseek(HZK, offset, SEEK_SET);
    fread(mat, 32, 1, HZK);//读取汉字的16*16点阵字模
    fclose(HZK);

    cout << type2str(image.type()) << endl;

    CvScalar color = CV_RGB(105,105,105);

    for (i = 0; i < 16; i++)  //显示点阵
    {
//        if (i == 8) {
//            continue;
//        }
        for (j = 0; j < 2; j++)
            for (k = 0; k < 8; k++) {
                if (mat[i][j] & (0x80 >> k)) {
                    printf("*");
//                    if (j == 1 && (k == 3 || k == 4)) {
//                        offset = -2;
//                    } else {
//                        offset = 0;
//                    }
                    Point P1(leftX + 3 * (j * 8 + k) + 1, leftY + 3 * i + 1);
                    circle(image, P1, 1, color, -1);

//                    Point P1((leftX + j * 8 + k) + 1, leftY + i + 1);
//                    circle(image, P1, 0, color);
//
//                    Point P1(leftX + 2 * (j * 8 + k) + 1, leftY + 2 * i + 1);
//                    circle(image, P1, 2, color, -1);
                }
                else
                    printf(" ");
            }
        printf("\n");
    }

    leftX = leftX + 3 * 2 * 8 + 1;

//    leftX = leftX + 2 * 8 + 1;

//    leftX = leftX + 2 * 2 * 8 + 1;

    return EXIT_SUCCESS;
}

int display16(char *incode, int len, int line) {
    int i, j, k;
    char mat[16][2];
    FILE *HZK = NULL;
    unsigned char qh, wh;
    unsigned long offset;

    qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
    wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
    offset = (94 * (qh - 1) + (wh - 1)) * 32; //得到汉字在HZK16字库中的字节偏移位置
    printf("区码：%d,位码：%d\n", qh, wh);

    if ((HZK = fopen("/home/sensetime/ClionProjects/Test/HZK16", "rb")) == NULL) {
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }
    fseek(HZK, offset, SEEK_SET);
    fread(mat, 32, 1, HZK);//读取汉字的16*16点阵字模
    fclose(HZK);

    cout << type2str(image.type()) << endl;

    CvScalar color = CV_RGB(105,105,105);

    for (i = 0; i < 16; i++)  //显示点阵
    {
        if (i == line) {
            continue;
        }
        for (j = 0; j < 2; j++)
            for (k = 0; k < 8; k++) {
                if (mat[i][j] & (0x80 >> k)) {
                    printf("*");
//                    if (j == 1 && (k == 3 || k == 4)) {
//                        offset = -2;
//                    } else {
//                        offset = 0;
//                    }
                    Point P1(leftX + 3 * (j * 8 + k) + 1 + offset , leftY + 3 * i + 1 + offset);
                    circle(image, P1, 1, color, -1);

//                    Point P1((leftX + j * 8 + k) + 1, leftY + i + 1);
//                    circle(image, P1, 0, color);
//
//                    Point P1(leftX + 2 * (j * 8 + k) + 1, leftY + 2 * i + 1);
//                    circle(image, P1, 2, color, -1);
                }
                else
                    printf(" ");
            }
        printf("\n");
    }

    leftX = leftX + 3 * 2 * 8 + 1;

//    leftX = leftX + 2 * 8 + 1;

//    leftX = leftX + 2 * 2 * 8 + 1;

    return EXIT_SUCCESS;
}

int display16(char *incode, int len, int colBase, int col1, int col2) {
    int i, j, k;
    char mat[16][2];
    FILE *HZK = NULL;
    unsigned char qh, wh;
    unsigned long offset;

    qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
    wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
    offset = (94 * (qh - 1) + (wh - 1)) * 32; //得到汉字在HZK16字库中的字节偏移位置
    printf("区码：%d,位码：%d\n", qh, wh);

    if ((HZK = fopen("/home/sensetime/ClionProjects/Test/HZK16", "rb")) == NULL) {
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }
    fseek(HZK, offset, SEEK_SET);
    fread(mat, 32, 1, HZK);//读取汉字的16*16点阵字模
    fclose(HZK);

    cout << type2str(image.type()) << endl;

    CvScalar color = CV_RGB(105,105,105);

    int bais = 0;

    for (i = 0; i < 16; i++)  //显示点阵
    {
//        if (i == 8) {
//            continue;
//        }
        for (j = 0; j < 2; j++)
            for (k = 0; k < 8; k++) {
                if (mat[i][j] & (0x80 >> k)) {
                    printf("*");
                    if (j == colBase && (k == col1 || k == col2)) {
                        bais = -2;
                    } else {
                        bais = 0;
                    }
                    Point P1(leftX + 3 * (j * 8 + k) + 1 + bais , leftY + 3 * i + 1 + bais);
                    circle(image, P1, 1, color, -1);

//                    Point P1((leftX + j * 8 + k) + 1, leftY + i + 1);
//                    circle(image, P1, 0, color);
//
//                    Point P1(leftX + 2 * (j * 8 + k) + 1, leftY + 2 * i + 1);
//                    circle(image, P1, 2, color, -1);
                }
                else
                    printf(" ");
            }
        printf("\n");
    }

    leftX = leftX + 3 * 2 * 8 + 1;

//    leftX = leftX + 2 * 8 + 1;

//    leftX = leftX + 2 * 2 * 8 + 1;

    return EXIT_SUCCESS;
}



int saveImage(Mat im, int x1, int y1, int x2, int y2) {
    Rect rect(x1, y1, x2, y2);
    Mat croppedImage = im(rect);

    char buffer1[300];
    sprintf(buffer1, "/alter/tttt/%d.jpg", image_count);
    imwrite(buffer1, croppedImage);

    image_count++;
}


int display48(char *incode, int len) {
    int i, j, k;
    char mat[48][6];
    FILE *HZK = NULL;
    unsigned char qh, wh;
    unsigned long offset;

    qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
    wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
    offset = (94 * (qh - 1) + (wh - 1)) * 288; //得到汉字在HZK16字库中的字节偏移位置
    printf("区码：%d,位码：%d\n", qh, wh);

    if ((HZK = fopen("/home/sensetime/ClionProjects/Test/HZK48S", "rb")) == NULL) {
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }
    fseek(HZK, offset, SEEK_SET);
    fread(mat, 288, 1, HZK);//读取汉字的16*16点阵字模
    fclose(HZK);

    cout << type2str(image.type()) << endl;

    CvScalar color = CV_RGB(105,105,105);

    for (i = 0; i < 48; i++)  //显示点阵
    {
        for (j = 0; j < 6; j++)
            for (k = 0; k < 8; k++) {
                if (mat[i][j] & (0x80 >> k)) {
                    printf("*");
//                    Point P1(leftX + 3 * (j * 8 + k) + 1, leftY + 3 * i + 1);
//                    circle(image, P1, 1, color, -1);

                    Point P1((leftX + j * 8 + k) + 1, leftY + i + 1);
                    circle(image, P1, 0, color);
//
//                    Point P1(leftX + 2 * (j * 8 + k) + 1, leftY + 2 * i + 1);
//                    circle(image, P1, 2, color, -1);
                }
                else
                    printf(" ");
            }
        printf("\n");
    }

//    leftX = leftX + 3 * 2 * 8 + 1;

    leftX = leftX + 48 + 1;

//    leftX = leftX + 2 * 2 * 8 + 1;

    return EXIT_SUCCESS;
}
