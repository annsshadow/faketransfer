# faketransfer-single-process
a file transfer with download and upload function

## Technology
* TCP

## Test Environment
* Two CentOS7.2 on the same host by VMware Workstation 12 Player
* 1 core i5-6500 CPU @ 3.20GHz and 1GB memory for each test VMware machine
* 7200 rpm hard disk

## Operation
### vm-one-server
* `cd the faketransfer floder`
* `make`
* `$./faketransfer-svr`

### vm-two-client
* `cd the faketransfer floder`
* `make`
* eg:`$./faketransfer-cli 10.26.99.58 upload file.txt /usr/local/fake-test/ /usr/local/fake-test/`
* `Usage: ./faketransfer-cli server-IP command <upload/download> filename absolute_source_file_path absolute_destination_file_path`

## Result
* Time cost between **35s ~ 42s**

## Execution Flow
* ![faketransfer-client-server](http://img-10063943.cos.myqcloud.com/faketransfer-client-server.png)
* ![faketransfer-download](http://img-10063943.cos.myqcloud.com/faketransfer-download.png)
* ![faketransfer-upload](http://img-10063943.cos.myqcloud.com/faketransfer-upload.png)
