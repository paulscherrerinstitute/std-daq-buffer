FROM centos:centos7

RUN yum -y install centos-release-scl epel-release && \
    yum -y update && \
    yum -y install devtoolset-9 git cmake3 wget zeromq-devel vim redis

SHELL ["scl", "enable", "devtoolset-9"]

