#!/bin/bash

cont=1;
while [ $cont -lt 12 ]; do
        ps uax | grep "haarp -c"| grep -v "grep" >/dev/null
        if [ $? -eq 1 ]; then
                echo "[`date`] Restart haarpcache .... " >> /var/log/haarp/error.log
                service haarp restart;
        fi
        let cont+=1;
        sleep 5;
done


