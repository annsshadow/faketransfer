# Test Environment
* Two CentOS7.2 on the same host by VMware Workstation 12 Player
* 1 core i5-6500 CPU @ 3.20GHz and 1GB memory for each test VMware machine
* 7200 rpm hard disk
* transfer a 3GB file

# faketransfer-single-process
a file transfer with download and upload function

## Technology-sp
* TCP

## Operation-sp
### vm-one-server
* `cd faketransfer/faketransfer-single-process`
* `make`
* `./faketransfer-svr`

### vm-two-client
* `cd the faketransfer floder`
* `make`
* eg:`./faketransfer-cli 10.26.99.58 upload file.txt /usr/local/fake-test/ /usr/local/fake-test/`
* `Usage: ./faketransfer-cli server-IP command <upload/download> filename absolute_source_file_path absolute_destination_file_path`

## Result-sp
* Time cost between **35s ~ 42s**

## Execution Flow
* ![faketransfer-client-server](http://img-10063943.file.myqcloud.com/faketransfer-client-server.png)
* ![faketransfer-download](http://img-10063943.file.myqcloud.com/faketransfer-download.png)
* ![faketransfer-upload](http://img-10063943.file.myqcloud.com/faketransfer-upload.png)

# faketransfer-multithread
a file transfer only with upload function

## Technology-mt
* TCP
* **epoll**
* **threadpool**
* **mmap**

## Operation-mt
### vm-one-server
* `cd faketransfer/faketransfer-multithread`
* `make`
* `./faketransfer-mt-svr`

### vm-two-client
* `cd faketransfer/faketransfer-multithread`
* `make`
* `./faketransfer-mt-cli`
* input the file name with absolute path

## Result-mt
* Time cost between **38s ~ 60s**

## Execution Flow
* ![faketransfer-multithread-client](http://img-10063943.file.myqcloud.com/faketransfer-multithread-client.png)
* ![faketransfer-multithread-server](http://img-10063943.file.myqcloud.com/faketransfer-multithread-server.png)
