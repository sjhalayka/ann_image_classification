#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "ffbpneuralnet.h"
using namespace std;


class header {
public:
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};

class pixel {
public:
    unsigned char r,g,b,a;
};


class tga_32bit_image
{
public:
    
    void MergeBytes(pixel *pxl, unsigned char *p, int bytes)
    {
        if (bytes == 4) {
            pxl->r = p[2];
            pxl->g = p[1];
            pxl->b = p[0];
            pxl->a = p[3];
        } else if (bytes == 3) {
            pxl->r = p[2];
            pxl->g = p[1];
            pxl->b = p[0];
            pxl->a = 0;
        } else if (bytes == 2) {
            pxl->r = (p[1] & 0x7c) << 1;
            pxl->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
            pxl->b = (p[0] & 0x1f) << 3;
            pxl->a = (p[1] & 0x80);
        }
    }
    

    void save(const char *const filename)
    {
        FILE *fptr;
        
        /* Write the result as a uncompressed TGA */
        if ((fptr = fopen(filename,"w")) == NULL) {
            fprintf(stderr,"Failed to open outputfile\n");
            exit(-1);
        }
        putc(0,fptr);
        putc(0,fptr);
        putc(2,fptr);                         /* uncompressed RGB */
        putc(0,fptr); putc(0,fptr);
        putc(0,fptr); putc(0,fptr);
        putc(0,fptr);
        putc(0,fptr); putc(0,fptr);           /* X origin */
        putc(0,fptr); putc(0,fptr);           /* y origin */
        putc((hdr.width & 0x00FF),fptr);
        putc((hdr.width & 0xFF00) / 256,fptr);
        putc((hdr.height & 0x00FF),fptr);
        putc((hdr.height & 0xFF00) / 256,fptr);
        putc(32,fptr);
        putc(0,fptr);
        for (size_t i=0;i<hdr.height*hdr.width;i++) {
            putc(pixels[i].b,fptr);
            putc(pixels[i].g,fptr);
            putc(pixels[i].r,fptr);
            putc(pixels[i].a,fptr);
        }
        
        fclose(fptr);
    }
    
    void load(const char *const filename)
    {
        int n=0,i,j;
        int bytes2read,skipover = 0;
        unsigned char p[5];
        FILE *fptr;
        
        /* Open the file */
        if ((fptr = fopen(filename,"r")) == NULL) {
            fprintf(stderr,"File open failed\n");
            return;
        }
        
        /* Display the header fields */
        hdr.idlength = fgetc(fptr);
        hdr.colourmaptype = fgetc(fptr);
        hdr.datatypecode = fgetc(fptr);
        fread(&hdr.colourmaporigin,2,1,fptr);
        fread(&hdr.colourmaplength,2,1,fptr);
        hdr.colourmapdepth = fgetc(fptr);
        fread(&hdr.x_origin,2,1,fptr);
        fread(&hdr.y_origin,2,1,fptr);
        fread(&hdr.width,2,1,fptr);
        fread(&hdr.height,2,1,fptr);
        hdr.bitsperpixel = fgetc(fptr);
        hdr.imagedescriptor = fgetc(fptr);

        pixels.resize(hdr.width*hdr.height);
        
        for (i=0;i<hdr.width*hdr.height;i++) {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
            pixels[i].a = 0;
        }
        
        /* Skip over unnecessary stuff */
        skipover += hdr.idlength;
        skipover += hdr.colourmaptype * hdr.colourmaplength;
        //    fprintf(stderr,"Skip over %d bytes\n",skipover);
        fseek(fptr,skipover,SEEK_CUR);
        
        /* Read the image */
        bytes2read = hdr.bitsperpixel / 8;
        while (n < hdr.width * hdr.height) {
            if (hdr.datatypecode == 2) {                     /* Uncompressed */
                if (fread(p,1,bytes2read,fptr) != bytes2read) {
                    //              fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                    return;
                }
                MergeBytes(&(pixels[n]),p,bytes2read);
                n++;
            } else if (hdr.datatypecode == 10) {             /* Compressed */
                if (fread(p,1,bytes2read+1,fptr) != bytes2read+1) {
                    //              fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                    return;
                }
                j = p[0] & 0x7f;
                MergeBytes(&(pixels[n]),&(p[1]),bytes2read);
                n++;
                if (p[0] & 0x80) {         /* RLE chunk */
                    for (i=0;i<j;i++) {
                        MergeBytes(&(pixels[n]),&(p[1]),bytes2read);
                        n++;
                    }
                } else {                   /* Normal chunk */
                    for (i=0;i<j;i++) {
                        if (fread(p,1,bytes2read,fptr) != bytes2read) {
                            //                      fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                            return;
                        }
                        MergeBytes(&(pixels[n]),p,bytes2read);
                        n++;
                    }
                }
            }
        }
     
        fclose(fptr);
    }
    
    void add_colour_noise(double scale)
    {
        
        for(size_t i = 0; i < pixels.size(); i++)
        {
            unsigned char noise = rand()%255;
            pixels[i].r = (pixels[i].r + noise*scale) / (1.0 + scale);
            noise = rand()%255;
            pixels[i].g = (pixels[i].g + noise*scale) / (1.0 + scale);
            noise = rand()%255;
            pixels[i].b = (pixels[i].b + noise*scale) / (1.0 + scale);
        }
    }
    
    vector<pixel> pixels;
    header hdr;
};





int main(int argc,char **argv)
{
    srand(time(0));

    const size_t image_width = 64;
    const size_t image_height = 64;
    const size_t bytes_per_pixel = 4;
    const size_t output_bits = 2;
    
    tga_32bit_image peacock_img;
    peacock_img.load("peacock.tga");

    tga_32bit_image dove_img;
    dove_img.load("dove.tga");

    tga_32bit_image flowers_img;
    flowers_img.load("flowers.tga");

    tga_32bit_image statue_img;
    statue_img.load("statue.tga");
    
    vector<size_t> HiddenLayers;
    HiddenLayers.push_back(sqrt(image_width*image_height*bytes_per_pixel*output_bits));
    FFBPNeuralNet NNet(image_width*image_height*bytes_per_pixel, HiddenLayers, output_bits);
    
    NNet.SetLearningRate(0.1);
    
    
    double max_error_rate = 0.01;
    long unsigned int max_training_sessions = 1000;
    
    double error_rate = 0.0;
    long unsigned int num_training_sessions = 0;
    


    
    
    // train network until the error rate goes below the maximum error rate
    // or we reach the maximum number of training sessions (which could be considered as "giving up")
    do
    {
        tga_32bit_image peacock_noise_img;
        tga_32bit_image dove_noise_img;
        tga_32bit_image flowers_noise_img;
        tga_32bit_image statue_noise_img;
        
        double noise_scale = 0.1;
        
        peacock_noise_img = peacock_img;
        peacock_noise_img.add_colour_noise(noise_scale);
        
        dove_noise_img = dove_img;
        dove_noise_img.add_colour_noise(noise_scale);
        
        flowers_noise_img = flowers_img;
        flowers_noise_img.add_colour_noise(noise_scale);
        
        statue_noise_img = statue_img;
        statue_noise_img.add_colour_noise(noise_scale);
        
        vector<double> data;

        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(peacock_noise_img.pixels[i].r / 255.0);
            data.push_back(peacock_noise_img.pixels[i].g / 255.0);
            data.push_back(peacock_noise_img.pixels[i].b / 255.0);
            data.push_back(peacock_noise_img.pixels[i].a / 255.0);
            
        }
        
        NNet.FeedForward(data);
        
        data.clear();
        data.push_back(0.0);
        data.push_back(0.0);
        
        error_rate = NNet.BackPropagate(data);


        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(dove_noise_img.pixels[i].r / 255.0);
            data.push_back(dove_noise_img.pixels[i].g / 255.0);
            data.push_back(dove_noise_img.pixels[i].b / 255.0);
            data.push_back(dove_noise_img.pixels[i].a / 255.0);
        }
        
        NNet.FeedForward(data);

        data.clear();
        data.push_back(1.0);
        data.push_back(0.0);
        
        error_rate += NNet.BackPropagate(data);
        

        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(flowers_noise_img.pixels[i].r / 255.0);
            data.push_back(flowers_noise_img.pixels[i].g / 255.0);
            data.push_back(flowers_noise_img.pixels[i].b / 255.0);
            data.push_back(flowers_noise_img.pixels[i].a / 255.0);
        }
        
        NNet.FeedForward(data);
        
        data.clear();
        data.push_back(0.0);
        data.push_back(1.0);
        
        error_rate += NNet.BackPropagate(data);
        
 
        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(statue_noise_img.pixels[i].r / 255.0);
            data.push_back(statue_noise_img.pixels[i].g / 255.0);
            data.push_back(statue_noise_img.pixels[i].b / 255.0);
            data.push_back(statue_noise_img.pixels[i].a / 255.0);
        }
        
        NNet.FeedForward(data);

        data.clear();
        data.push_back(1.0);
        data.push_back(1.0);
        
        error_rate += NNet.BackPropagate(data);
 
        
        error_rate /= 4.0;


        num_training_sessions++;

        cout << num_training_sessions << " " << error_rate << endl << endl;
    }
    while(error_rate >= max_error_rate && num_training_sessions < max_training_sessions);
    
    
    return 0;
}















