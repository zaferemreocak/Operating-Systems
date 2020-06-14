# -*- coding: utf-8 -*-
"""
Created on Sun Jun 14 19:28:09 2020

@author: Emre
"""

import sys
import _thread
from queue import Queue



#   reader thread
def read(rr_buffer):
    
    if(len(sys.argv) != 0):
        with open(sys.argv[1]) as input_file:
            for line in input_file:
#                print("Reader-Thread\t{}\t-".format(line[:63]))
                output_file.write("Reader-Thread\t{}\t-".format(line[:63]))
                rr_buffer.put(line[:63])
            
            #   exit signal
            rr_buffer.put("INTERRUPT_SIG")
                
#   replacer thread
def replace(rr_buffer, ru_buffer):
    
    while(True):
        line = rr_buffer.get()
        if(line == "INTERRUPT_SIG"):
            break
        replaced_line = line.replace(" ", "-")
#        print("Replacer-Thread\t{}\t{}".format(line, replaced_line))
        output_file.write("Replacer-Thread\t{}\t{}".format(line, replaced_line))
        ru_buffer.put(replaced_line)

    #   exit signal
    ru_buffer.put("INTERRUPT_SIG")

#   upper thread
def upper(ru_buffer, uw_buffer):
    
    while(True):
        line = ru_buffer.get()
        if(line == "INTERRUPT_SIG"):
            break
        upper_line = line.upper()
#        print("Upper-Thread\t{}\t{}".format(line, upper_line))
        output_file.write("Upper-Thread\t{}\t{}".format(line, upper_line))
        uw_buffer.put(upper_line)
        
    #   exit signal
    uw_buffer.put("INTERRUPT_SIG")
        
#   writer thread
def writer(uw_buffer, wm_buffer):
    
    count = 0
    while(True):
        line = uw_buffer.get()
        if(line == "INTERRUPT_SIG"):
            break
        count += 1
        output_file.write("Writer-Thread\t{}\t-".format(line))
        
    #   exit signal
    print("# of lines: {}".format(count))
    wm_buffer.put("INTERRUPT_SIG")
        
#   main
try:
    output_file = open("output.txt", "w")
    rr_buffer = Queue()
    ru_buffer = Queue()
    uw_buffer = Queue()
    wm_buffer = Queue()
    reader = _thread.start_new_thread(read, (rr_buffer,))
    replacer = _thread.start_new_thread(replace, (rr_buffer,ru_buffer,))
    upper = _thread.start_new_thread(upper, (ru_buffer, uw_buffer,))
    writer = _thread.start_new_thread(writer, (uw_buffer, wm_buffer,))
except:
    print("Error: unable to start thread")
    
while(True):
    if(wm_buffer.get() == "INTERRUPT_SIG"):
        output_file.close()
        break