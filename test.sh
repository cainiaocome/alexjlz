#!/usr/bin/env bash

current_runlevel=`runlevel | cut -d ' ' -f 2`
current_rc_d="/etc/rc$current_runlevel.d/"
inject_file=`ls $current_rc_d | grep 'cron' | tail -n 1`
inject_file_real_path=$current_rc_d$inject_file

cat $inject_file_real_path | grep '^start)'
