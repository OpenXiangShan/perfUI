# This Python file uses the following encoding: utf-8

import re
import os

all_perf_counters = dict()

def Add(a, b , para):
    print("111232");
    return ("hello", 10000)

def parseLine(line):
    perf_re = re.compile(r'.*\[PERF \]\[time=\s*\d*\] ((\w*(\.|))*): (\w*)\s*,\s*(\d*)')
    global all_perf_counters
    all_perf_counters = dict()
    perf_match = perf_re.match(line)
    if perf_match:
        perf_name = ".".join([str(perf_match.group(1)), str(perf_match.group(4))])
        perf_value = str(perf_match.group(5))
        all_perf_counters[perf_name] = perf_value
        return (perf_name, perf_value)
    return ("", 0)

def parseTime(line):
    perf_re = re.compile(r'.*\[PERF \]\[time=\s*(\d*)\]*')
    perf_match = perf_re.match(line)
    global all_perf_counters
    print(all_perf_counters)
    if perf_match:
        time = str(perf_match.group(1))
        print(time)
        return time
    return ""

if __name__ == "__main__":
    pass

