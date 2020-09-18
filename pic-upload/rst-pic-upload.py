#!/usr/bin/python
# -*-coding=utf-8 -*-

import json
import urllib
import urllib2
import requests
import pickle
import os
import base64
import time

import hashlib
import time
import sys, getopt

import logging
import datetime

logger = logging.getLogger()
logger.setLevel(logging.INFO)
# 创建一个FileHandler，将日志输出到文件
log_file = '/home/openailab/log/ledpic_%s.log' % datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d')
file_handler = logging.FileHandler(log_file)
# 设置此Handler的最低日志级别
file_handler.setLevel(logging.INFO)
# 设置此Handler的日志输出字符串格式
log_formatter = logging.Formatter('%(asctime)s[%(levelname)s]: %(message)s')
file_handler.setFormatter(log_formatter)

# 创建一个StreamHandler，将日志输出到Stream，默认输出到sys.stderr
stream_handler = logging.StreamHandler()
stream_handler.setLevel(logging.INFO)

# 将不同的Handler添加到logger中，日志就会同时输出到不同的Handler控制的输出中
# 注意如果此logger在之前使用basicConfig进行基础配置，因为basicConfig会自动创建一个Handler，所以此logger将会有3个Handler
# 会将日志同时输出到3个Handler控制的输出中
logger.addHandler(file_handler)
logger.addHandler(stream_handler)

app_secret = "B7CUilppXUUQ4CJnZEZELKGPEhIn8iwN"
SN = "100002"
laterSecond = 30

try:
    opts, args = getopt.getopt(sys.argv[1:], "hs:l:", ["SN=", "later="])
except getopt.GetoptError:
    logger.info('XXX.py -s <SN> -l <laterSecond>')
    sys.exit(2)
for opt, arg in opts:
    if opt == '-h':
        logger.info('XXX.py -s <SN> -l <laterSecond>')
        sys.exit()
    elif opt in ("-s", "--SN"):
        SN = arg
    elif opt in ("-l", "--later"):
        laterSecond = int(arg)

logger.info('SN:'+ SN)
logger.info('laterSecond:' + str(laterSecond))

def data_processing(data):
    """
    data: 需要签名的数据，字典类型
    return: 处理后的字符串， 格式为：参数名称参数值
    """
    dataList = []
    for k in sorted(data):
        dataList.append(("%s%s" %(k, data[k])))
    return "".join(dataList)

def md5_sign(data,app_secret):
    data = app_secret+data_processing(data)+app_secret
    md5 = hashlib.md5()
    md5.update(data.encode("utf-8"))
    return md5.hexdigest()
    
def post(server_url, params):
    headers = {'Content-type':'application/json'}
    data = json.dumps(params)
    #print data
    try:
        request = urllib2.Request(server_url, data=data, headers=headers)
    except:
        logger.error("urllib2 Request error!")
    try:
        return json.loads(urllib2.urlopen(request, timeout=20).read())
    except urllib2.HTTPError, err:
        return err.code
    except urllib2.URLError, err:
        return err


def process_image(server_url, image_path):
    """

    """
    exceptionFlag = 0
    elements = image_path.split('_')
    print(elements)
    proof_path = '_'.join([elements[0],elements[1],elements[2],"0.jpg"])
    mark_path = '_'.join([elements[0],elements[1],elements[2],"1.jpg"])
    try:
        proof_file = open(proof_path, "rb")
    except:
        logger.warning("open mark_file failed!")
        exceptionFlag = 1
    try:
        mark_file = open(mark_path, "rb")
    except:
        logger.warning("open proof_file failed!")
        exceptionFlag = 1
        
    if 0 == exceptionFlag :
        proof_image = base64.b64encode(proof_file.read())
        mark_image = base64.b64encode(mark_file.read())
        
        params = {
            "app_key": "uRRurLUJ8cSENTVM",
            "timestamp": "2019-10-07 15:56:32",
            "format": "json",
            "app_id": "uRRurLUJ8cSENTVM",
            "serial_no": SN,
            "display_index": "1",
            "result": elements[1],
            "confidence": elements[2],
            "proof_image":proof_image,
            "mark_image":mark_image
            }
        params.update({"sign":md5_sign(params,app_secret)})
        logger.debug(str(params))
        logger.info(post(server_url, params))
    try:
        os.remove(proof_path)
        os.remove(mark_path)
    except:
        logger.error("error occured with os.remove()")

def process_heartbeat(server_url):
    """

    """
    params = {
        "app_key": "uRRurLUJ8cSENTVM",
        "timestamp": "2019-10-07 15:56:32",
        "format": "json",
        "app_id": "uRRurLUJ8cSENTVM",
        "serial_no": SN,
        "display_index": "1",
        "result": 0,
        "confidence": 0
        }
    params.update({"sign":md5_sign(params,app_secret)})
    logger.info(post(server_url, params))

if __name__ == "__main__":
    
    url = "https://api.gateway.zqpxb.cn/api/faultService/v1/gather/collectCheckLog"
    #app_secret = "B7CUilppXUUQ4CJnZEZELKGPEhIn8iwN"
    while 1:
        isNormal = 1
        #for parent, dirnames, filenames in os.walk(os.getcwd()):
        for parent, dirnames, filenames in os.walk("/home/openailab/process/pic-upload"):
            for filename in filenames:
                # filename format: 时间_类别_置信度_是否标注.jpg 
                # filename example: 1570460400_2_0.91_0.jpg ,1570460400_2_0.91_1.jpg 
                if filename.endswith('1.jpg'):
                    print(filename)
                    time.sleep(1)  # ensure file writing completed
                    process_image(url, parent+'/'+filename)
                    isNormal = 0
        if isNormal :
            process_heartbeat(url)
        time.sleep(laterSecond)

    
