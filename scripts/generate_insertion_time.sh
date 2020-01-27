#!/bin/bash

build/riski -pcap_feed /hdd/iex/hist/deep/20200114_DEEP1.0.pcap | grep "insertion time" | awk '{print $NF}' > insertion_time_data.dat
