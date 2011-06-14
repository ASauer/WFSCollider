#!/bin/sh

cmake -D standalone:string=WFSCollider -DSC_QT=OFF -DWFS_DEV=ON -DCMAKE_BUILD_TYPE=Release ..
