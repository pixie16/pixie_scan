#!/bin/bash

ln -s -f config/isolde/is599-600/vcal.xml Config.xml

for file in `ls data/isolde/oct/is599/052k/ data/isolde/aug/is599/052k/ -R | awk '/:$/&&f{s=$0;f=0} /:$/&&!f{sub(/:$/,"");s=$0;f=1;next} NF&&f{ print s"/"$0 }'`
do
    if [ $file == "data/isolde/aug/is599/052k//is599_52k_03.ldf" ]
    then
	continue
    fi
    if [ $file == "data/isolde/oct/is599/052k//IS599Oct_A052__HRS_07.ldf" ]
    then
	continue
    fi
    if [ $file == "data/isolde/oct/is599/052k//IS599_A052_01.ldf" ]
    then
	continue
    fi

    name=`basename $file .ldf`
    cmd="file $file\ngo\nend"
    echo -e $cmd | ./pixie_ldf_c $name &
done
