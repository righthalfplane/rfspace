import SoapySDR
from SoapySDR import * #SOAPY_SDR_ constants
import numpy as np
from scipy import signal
import sounddevice as sd
import time
import math

#enumerate devices
results = SoapySDR.Device.enumerate()
for result in results: print(result)

#create device instance
#args can be user defined or from the enumeration result
args = dict(driver="rtlsdr")
sdr = SoapySDR.Device()

#query device info
print(sdr.listAntennas(SOAPY_SDR_RX, 0))
print(sdr.listGains(SOAPY_SDR_RX, 0))
freqs = sdr.getFrequencyRange(SOAPY_SDR_RX, 0)
for freqRange in freqs: print(freqRange)

setting_info_list = sdr.getSettingInfo()
print(" ")   
for setting_info in setting_info_list:
    print(f"Key: {setting_info.key}")
    print(f"Name: {setting_info.name}")
    print(f"Description: {setting_info.description}")
    print(f"Type: {setting_info.type}")
    print(f"Units: {setting_info.units}")
    print(f"Value: {setting_info.value}")
    print(" ")
    
sdr.writeSetting("rfnotch_ctrl","1")

sampleRate=2.0e6

audioRate=48000

fc=101.6e6
f=101.5e6

#fc=0.76e6
#f=0.6e6

#fc=0.76e6
#f=0.6e6

#apply settings
sdr.setSampleRate(SOAPY_SDR_RX, 0, sampleRate)
#sdr.setBandwidth(SOAPY_SDR_RX, 0, sampleRate)
sdr.setFrequency(SOAPY_SDR_RX, 0, fc)

global buff1,buff2,buff3,rxStream

#setup a stream (complex floats)
rxStream = sdr.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32)
sdr.activateStream(rxStream) #start streaming

MTU=sdr.getStreamMTU(rxStream);
print("MTU ",MTU)

fpwd= open("0rx_fm_101500000_2000000_fc.raw",'wb')

BUFF_SIZE=50000

#create a re-usable buffer for rx samples
buff1 = np.array([0]*BUFF_SIZE, np.complex64)
buff2 = np.array([0]*BUFF_SIZE, np.complex64)
buff3 = np.array([0]*BUFF_SIZE, np.complex64)

t = np.arange(BUFF_SIZE) / sampleRate
freq_shift = np.exp(-1j * 2 * np.pi * (f-fc) * t)
freq_shift = freq_shift.astype(np.complex64)

def receive():
    global buff1,buff2,rxStream,sdr
    nc=len(buff1)
    nl=0
    while True:
        sr = sdr.readStream(rxStream, [buff2], nc)
        if sr.ret != nc:
            if sr.ret <= 0:
                #print("-1 return")
                break
            buff1[nl:nl+sr.ret]=buff2[0:sr.ret]
            nc=nc-sr.ret
            nl=nl+sr.ret
            if nc == 0:
                break
            continue
        buff1[nl:nl+sr.ret]=buff2[0:sr.ret]
        break	
        
try:
	
    def audio_callback(outdata, frames, time2, status):
        global buff1
        		
        if status:
            print(status)
	
        receive()
        
        dec=int(0.5*sampleRate/audioRate);
        
        audio_data=buff1*freq_shift;
        
        fpwd.write(audio_data)
 
        audio_data = signal.decimate(audio_data, dec)
                
        audio_data = np.diff(np.unwrap(np.angle(audio_data)))
                         
        audio_data=audio_data-np.average(audio_data)
        
        audio_data=audio_data/np.max(audio_data)
        
        resampled_audio = np.interp(np.linspace(0, 1, frames), np.linspace(0, 1, len(audio_data)), audio_data)
                   
        outdata[:] = np.int16(resampled_audio * 32767)[:, None] #convert to mono, scale to int16 range

    with sd.OutputStream(samplerate=audioRate, channels=1, callback=audio_callback, dtype='int16', blocksize=int(audioRate*BUFF_SIZE/sampleRate)):
        print("Listening... Press Ctrl+C to stop")
        while True:
            time.sleep(0.1)
            	
    print("Normal Stop")
except KeyboardInterrupt:
    print("Stopped")
except Exception as e:
    print(f"Error: {e}")
finally:
    print("done")

#shutdown the stream
fpwd.close()
sdr.deactivateStream(rxStream) #stop streaming
sdr.closeStream(rxStream)

