#!/bin/bash

for file in `ls data/isolde/oct/is599/052k/ -R | awk '/:$/&&f{s=$0;f=0} /:$/&&!f{sub(/:$/,"");s=$0;f=1;next} NF&&f{ print s"/"$0 }'`
do
    if [ $file == "data/isolde/oct/is599/052k//IS599Oct_A052__HRS_07.ldf" ]
    then
	continue
    fi
    if [ $file == "data/isolde/oct/is599/052k//IS599_A052_01.ldf" ]
    then
	continue
    fi

    name=`basename $file .ldf`-gcal
#    rm -rf $name.*

#    cfg=`config/isolde/is599-600/gcal-oct.xml`
#    ln -s -f $cfg Config.xml
#    cmd="file $file\ngo\nend"
#    echo -e $cmd | ./pixie_ldf_c $name
done
