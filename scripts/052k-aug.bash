#!/bin/bash

his="052k-aug"
rm -rf $his.*

for file in `ls data/isolde/aug/is599/052k/ -R | awk '/:$/&&f{s=$0;f=0} /:$/&&!f{sub(/:$/,"");s=$0;f=1;next} NF&&f{ print s"/"$0 }'`
do
    if [ $file == "data/isolde/aug/is599/052k//is599_52k_03.ldf" ]
    then
	continue
    fi

    name=`basename $file .ldf`
    cfg=`find ./ -name "$name.xml"`
    ln -s -f $cfg Config.xml
    cmd="file $file\ngo\nend"
    echo -e $cmd | ./pixie_ldf_c $his
done