#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "ffbpneuralnet.h"
#include "tga_image.h"

using namespace std;






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
    
    NNet.SaveToFile("network.bin");

    
    FFBPNeuralNet NNet2("network.bin");
    
    
    
    size_t num_tests = 100;
    size_t num_successes = 0;
    size_t num_failures = 0;
    
    for(size_t i = 0; i < num_tests; i++)
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
        
        NNet2.FeedForward(data);
        NNet2.GetOutputValues(data);

        if(floor(data[1] + 0.5) == 0 && floor(data[0] + 0.5) == 0)
            num_successes++;
        else
            num_failures++;

        
        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(dove_noise_img.pixels[i].r / 255.0);
            data.push_back(dove_noise_img.pixels[i].g / 255.0);
            data.push_back(dove_noise_img.pixels[i].b / 255.0);
            data.push_back(dove_noise_img.pixels[i].a / 255.0);
            
        }
        
        NNet2.FeedForward(data);
        NNet2.GetOutputValues(data);
        
        if(floor(data[1] + 0.5) == 0 && floor(data[0] + 0.5) == 1)
            num_successes++;
        else
            num_failures++;

        
        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(flowers_noise_img.pixels[i].r / 255.0);
            data.push_back(flowers_noise_img.pixels[i].g / 255.0);
            data.push_back(flowers_noise_img.pixels[i].b / 255.0);
            data.push_back(flowers_noise_img.pixels[i].a / 255.0);
            
        }
        
        NNet2.FeedForward(data);
        NNet2.GetOutputValues(data);
        
        if(floor(data[1] + 0.5) == 1 && floor(data[0] + 0.5) == 0)
            num_successes++;
        else
            num_failures++;

        
        data.clear();
        
        for(size_t i = 0; i < image_width*image_height; i++)
        {
            data.push_back(statue_noise_img.pixels[i].r / 255.0);
            data.push_back(statue_noise_img.pixels[i].g / 255.0);
            data.push_back(statue_noise_img.pixels[i].b / 255.0);
            data.push_back(statue_noise_img.pixels[i].a / 255.0);
            
        }
        
        NNet2.FeedForward(data);
        NNet2.GetOutputValues(data);
        
        if(floor(data[1] + 0.5) == 1 && floor(data[0] + 0.5) == 1)
            num_successes++;
        else
            num_failures++;
        
        cout << i + 1 << endl;
    }

    cout << "Success rate: " << 100.0*static_cast<double>(num_successes) / static_cast<double>(num_tests*4) << "%" << endl;
    
    return 0;
}















