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
 * Copyright (c) 2019, Nut.AI
 * Author: davche@163.com
 *
 */

#include "ledssd.hpp"

#include "ipc_rtsp.hpp"

const string wintitle = "LED-Detection-rtsp";

const string ipcUrl="rtsp://192.168.1.108/cam/realmonitor?channel=1&subtype=0";
//const string ipcUrl="rtsp://192.168.1.108/video1";
const string ipcUser="admin";
const string ipcPassword="admin000";



typedef struct _VideoDecoder {
    ipcCamera* ipc;
    Mat* image;
} VideoDecoder;


static void usage(char **argv)
{
    printf(
        "Usage: %s [Options]\n\n"
        "Options:\n"
        "-w, --width                  Destination images's width\n"
        "-h, --height                 Destination images's height\n"
        "-r, --rotate                 Image rotation degree, the value should be 90, 180 or 270\n"
        "-D, --display                display for debug\n"
        "-V, --vflip                  Vertical Mirror\n"
        "-H, --hflip                  Horizontal Mirror\n"
        "-c, --crop                   Crop, format: x,y,w,h\n"
        "-p, --proto                  proto_file, format: xx/xx.prototxt\n"
        "-m, --model                  model_file, format: xx/xx.caffemodel\n"
        "-d, --decoder                decoder, format: 10\n"
        "-l, --writeLimit             image write limit time: 0~100(0 is forbid write)\n"
        "\n",
        argv[0]);
}

static const char *short_options = "d:w:h:r:DVHc:p:m:l:";

static struct option long_options[] = {
    {"decoder", required_argument, NULL, 'd'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"rotate", required_argument, NULL, 'r'},
    {"display", no_argument, NULL, 'D'},
    {"vflip", no_argument, NULL, 'V'},
    {"hflip", no_argument, NULL, 'H'},
    {"crop", required_argument, NULL, 'c'},
    {"proto", required_argument, NULL, 'p'},
    {"model", required_argument, NULL, 'm'},
    {"writeLimit", required_argument, NULL, 'l'},
    {NULL, 0, NULL, 0}
};


void *onVideoDecode(void *data)
{
    VideoDecoder *dec = (VideoDecoder*)data;
    ipcCamera *ipc = dec->ipc;
    Mat *mat = dec->image;
    DecFrame *frame = NULL;

    unsigned long long t1, t2;

    while(1) {
        frame = ipc->dequeue();
        if(frame != NULL) {
            t1 = ipc->microTime();
            ipc->rgaProcess(frame, V4L2_PIX_FMT_RGB24, mat);
            t2 = ipc->microTime();
            usleep(2000);
            printf("rga time: %llums\n", (t2 - t1)/1000);
            ipc->freeFrame(frame);
        }
        usleep(1000);
    }

    return NULL;
}



static void parse_crop_parameters(char *crop, __u32 *cropx, __u32 *cropy, __u32 *cropw, __u32 *croph)
{
    char *p, *buf;
    const char *delims = ".,";
    __u32 v[4] = {0,0,0,0};
    int i = 0;

    buf = strdup(crop);
    p = strtok(buf, delims);
    while(p != NULL) {
        v[i++] = atoi(p);
        p = strtok(NULL, delims);

        if(i >=4)
            break;
    }

    *cropx = v[0];
    *cropy = v[1];
    *cropw = v[2];
    *croph = v[3];
}

int main(int argc, char **argv)
{
    sleep(10);
    int ret, c, r;
    pthread_t id;

    Mat image;
    struct timeval t0, t1;

    __u32 width = 800, height = 480;
    RgaRotate rotate = RGA_ROTATE_NONE;
    __u32 cropx = 0, cropy = 0, cropw = 0, croph = 0;
    int vflip = 0, hflip = 0;
    int display = 0;
    DecodeType type=DECODE_TYPE_H264;
    //DecodeType type=DECODE_TYPE_H265;
    int writeLimit = 0;
    
    std::string proto_file;
    std::string model_file;
    std::string image_file;
    std::string save_name = "save.jpg";
    const char* device = nullptr;

    VideoDecoder* decoder=(VideoDecoder*)malloc(sizeof(VideoDecoder));

    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
        case 'd':
            if(strncmp(optarg, "h264", 5) == 0)
                type = DECODE_TYPE_H264;
            if(strncmp(optarg, "h265", 5) == 0)
                type = DECODE_TYPE_H265;
            break;
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        case 'r':
            r = atoi(optarg);
            switch(r) {
            case 0:
                rotate = RGA_ROTATE_NONE;
                break;
            case 90:
                rotate = RGA_ROTATE_90;
                break;
            case 180:
                rotate = RGA_ROTATE_180;
                break;
            case 270:
                rotate = RGA_ROTATE_270;
                break;
            default:
                printf("roate %d is not supported\n", r);
                return -1;
            }
            break;
        case 'D':
            display = 1;
            break;
        case 'V':
            vflip = 1;
            break;
        case 'H':
            hflip = 1;
            break;
        case 'c':
            parse_crop_parameters(optarg, &cropx, &cropy, &cropw, &croph);
            break;
        case 'p':
            proto_file = optarg;
            break;
        case 'm':
            model_file = optarg;
            break;
        case 'l':
            writeLimit = atoi(optarg);
            break;
        default:
            usage(argv);
            return 0;
        }
    }

    printf("decoder=%u, width = %u, height = %u, rotate = %u, vflip = %d, hflip = %d, crop = [%u, %u, %u, %u]\n",
           type, width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph);


    if(display) fcv::namedWindow(wintitle);

    ipcCamera *ipc = new ipcCamera(width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph);

    decoder->ipc = ipc;
    decoder->image = &image;

    RtspClient client(ipcUrl, ipcUser, ipcPassword);
    ret = ipc->init(type);

    image.create(cv::Size(width, height), CV_8UC3);



    if(ret < 0)
        return ret;
    ret = pthread_create(&id, NULL, onVideoDecode, decoder);
    if(ret < 0) {
        printf("pthread_create failed\n");
        return ret;
    }

    client.setDataCallback(std::bind(&ipcCamera::onStreamReceive, ipc, std::placeholders::_1, std::placeholders::_2));
    client.enable();
    

    // check Detection model
    if(model_file.empty())
    {
        model_file = DEF_MODEL;
        std::cout << "model file not specified,using " << model_file << " by default\n";
    }
    
    // init tengine
    if(init_tengine() < 0)
    {
        std::cout << " init tengine failed\n";
        return 1;
    }
    if(request_tengine_version("0.9") != 1)
    {
        std::cout << " request tengine version failed\n";
        return 1;
    }
    graph_t graph = create_graph(nullptr, "tengine", model_file.c_str());

    if(graph == nullptr)
    {
        std::cout << "Create graph failed\n";
        std::cout << " ,errno: " << get_tengine_errno() << "\n";
        return 1;
    }

    if(device != nullptr)
    {
        set_graph_device(graph, device);
    }

    // dump_graph(graph);

    // input
    int img_h = 600;
    int img_w = 600;
    int img_size = img_h * img_w * 3;
    float* input_data = ( float* )malloc(sizeof(float) * img_size);

    int node_idx = 0;
    int tensor_idx = 0;
    tensor_t input_tensor = get_graph_input_tensor(graph, node_idx, tensor_idx);
    if(input_tensor == nullptr)
    {
        std::printf("Cannot find input tensor,node_idx: %d,tensor_idx: %d\n", node_idx, tensor_idx);
        return -1;
    }

    int dims[] = {1, 3, img_h, img_w};
    set_tensor_shape(input_tensor, dims, 4);
    ret = prerun_graph(graph);
    if(ret != 0)
    {
        std::cout << "Prerun graph failed, errno: " << get_tengine_errno() << "\n";
        return 1;
    }
    
    // image write and interval time setup
    if(writeLimit > 0){
        init_time(1,writeLimit);
    }else{
        init_time(0,0);
    }
    // loop from rtsp images
    int testCNT = 0;
    while(1) {
        gettimeofday(&t0, NULL);
        if(display) imshow(wintitle, image, NULL);
        
        get_input_data_ssd(&image, input_data, img_h, img_w);
        set_tensor_buffer(input_tensor, input_data, img_size * 4);
        ret = run_graph(graph, 1);
        if(ret != 0)
        {
            std::cout << "Run graph failed, errno: " << get_tengine_errno() << "\n";
            return 1;
        }
        
        tensor_t out_tensor = get_graph_output_tensor(graph, 0, 0);    //"detection_out");
        int out_dim[4];
        ret = get_tensor_shape(out_tensor, out_dim, 4);
        if(ret <= 0)
        {
            std::cout << "get tensor shape failed, errno: " << get_tengine_errno() << "\n";
            return 1;
        }
        float* outdata = ( float* )get_tensor_buffer(out_tensor);
        int num = out_dim[1];
        float show_threshold = 0.9;

        post_process_ssd(&image, show_threshold, outdata, num, save_name);
        if(display) fcv::imshow(wintitle, image, NULL);
        
        // show elapse time
        gettimeofday(&t1, NULL);
        fcv::waitKey(1);
        long elapse = ((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
        cout <<"bladecv::imshow costs "<< elapse <<" miliseconds"<< endl;
        
        // release out_tensor
        release_graph_tensor(out_tensor);
        
        // test break
        if(testCNT == 500000)
            break;
        //testCNT++;
	//sleep(1);
    }
    
    /* release and close tengine */
    release_graph_tensor(input_tensor);
    ret = postrun_graph(graph);
    if(ret != 0)
    {
        std::cout << "Postrun graph failed, errno: " << get_tengine_errno() << "\n";
        return 1;
    }
    free(input_data);
    destroy_graph(graph);
    release_tengine();
    
    pthread_join(id, NULL);
	image.release();
	delete ipc;
	free(decoder);
    return 0;
}
