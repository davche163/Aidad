/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2018, Open AI Lab
 * Author: davche@163.com
 */
#include "ledssd.hpp"

struct timeval t0, t1;
int writeStopTime;
int writeFlag;  //0:image file write off, 1:write on, 2:writeing

struct Box
{
    float x0;
    float y0;
    float x1;
    float y1;
    int class_idx;
    float score;
};

void init_time(int writeflag,int stoptime)
{
    gettimeofday(&t0, NULL);
    gettimeofday(&t1, NULL);
    t1.tv_sec = 0;
    writeFlag = writeflag;
    writeStopTime = stoptime;
}

void get_input_data_ssd(fcv::Mat* imgptr, float* input_data, int img_h, int img_w)
{
    //if(img.empty())
    //{
    //    std::cerr << "Failed to read image file " << image_file << ".\n";
    //    return;
    //}
    fcv::Mat img = *imgptr;
    cv::resize(img, img, fcv::Size(img_h, img_w));
    img.convertTo(img, CV_32FC3);
    float* img_data = ( float* )img.data;
    int hw = img_h * img_w;

    float mean[3] = {127.5, 127.5, 127.5};
    for(int h = 0; h < img_h; h++)
    {
        for(int w = 0; w < img_w; w++)
        {
            for(int c = 0; c < 3; c++)
            {
                input_data[c * hw + h * img_w + w] = 0.007843 * (*img_data - mean[c]);
                img_data++;
            }
        }
    }
}

void post_process_ssd(fcv::Mat* imgptr, float threshold, float* outdata, int num, std::string& save_name)
{
    const char* class_names[] = {"background", "aeroplane", "bicycle",   "bird",   "boat",        "bottle",
                                 "bus",        "car",       "cat",       "chair",  "cow",         "diningtable",
                                 "dog",        "horse",     "motorbike", "person", "pottedplant", "sheep",
                                 "sofa",       "train",     "tv",  "anamorphose", "blue screen", "black screen", "black block"};

    fcv::Mat img = *imgptr;
    int raw_h = img.size().height;
    int raw_w = img.size().width;
    std::vector<Box> boxes;
    int line_width = raw_w * 0.005;
    printf("detect result num: %d \n", num);
    for(int i = 0; i < num; i++)
    {
        if(outdata[1] >= threshold && outdata[0] > 20)
        {
            Box box;
            box.class_idx = outdata[0];
            box.score = outdata[1];
            box.x0 = outdata[2] * raw_w;
            box.y0 = outdata[3] * raw_h;
            box.x1 = outdata[4] * raw_w;
            box.y1 = outdata[5] * raw_h;
            boxes.push_back(box);
            printf("%s\t:%.0f%%\n", class_names[box.class_idx], box.score * 100);
            printf("BOX:( %g , %g ),( %g , %g )\n", box.x0, box.y0, box.x1, box.y1);
        }
        outdata += 6;
    }

    if (( int )boxes.size() > 0 && writeFlag==1){
        gettimeofday(&t0, NULL);
        if (t1.tv_sec==0 || (t0.tv_sec - t1.tv_sec)>writeStopTime){
            writeFlag++;
            fcv::imwrite("/home/openailab/process/pic-upload/" + std::to_string(t0.tv_sec) + "_" + std::to_string(boxes[0].class_idx) + "_" + std::to_string(boxes[0].score) + "_0.jpg", img);
        }
    }

    for(int i = 0; i < ( int )boxes.size(); i++)
    {
        Box box = boxes[i];
        cv::rectangle(img, cv::Rect(box.x0, box.y0, (box.x1 - box.x0), (box.y1 - box.y0)), cv::Scalar(255, 255, 0),
                      line_width);
        std::ostringstream score_str;
        score_str << box.score;
        std::string label = std::string(class_names[box.class_idx]) + ": " + score_str.str();
        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::rectangle(img,
                      cv::Rect(cv::Point(box.x0, box.y0 - label_size.height),
                               cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 0), CV_FILLED);
        cv::putText(img, label, cv::Point(box.x0, box.y0), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    if (writeFlag == 2){
        fcv::imwrite("/home/openailab/process/pic-upload/" + std::to_string(t0.tv_sec) + "_" + std::to_string(boxes[0].class_idx) + "_" + std::to_string(boxes[0].score) + "_1.jpg", img);
        gettimeofday(&t1, NULL);
        writeFlag--;
    }

    //std::cout << "======================================\n";
    //std::cout << "[DETECTED IMAGE SAVED]:\t" << save_name << "\n";
    //std::cout << "======================================\n";
}


