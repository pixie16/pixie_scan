#!/bin/bash

name="test53n"
data="/home/pixie16/isoldeData/is599/053k/"
confs="/home/pixie16/svp/OptionalConfigurations/is599"

make && rm -f $name.*

ln -s -f $confs/Config.xml.is599_54k_01 Config.xml
cmd="zero\nban 051k.ban\nfile $data/is599_53k_08.ldf\ngo\nend"
echo -e $cmd | ./pixie_ldf_c $name

cmd="nban 051k.ban\nfile $data/is599_53k_09.ldf\ngo\nend"
echo -e $cmd | ./pixie_ldf_c $name
