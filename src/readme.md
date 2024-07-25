# FeliCa Library

PaSoRi §Ú π§√§∆ FeliCa §À•¢•Ø•ª•π§π§Î§ø§·§Œ•È•§•÷•È•Í§«§π°£

‘îºö§œ http://felicalib.tmurakam.org §Ú≤Œ’’§∑§∆§Ø§¿§µ§§°£

## FelicaDump

### [Detours](https://github.com/microsoft/Detours)
 &emsp; &emsp;X64 some sample can not been compiled. it works fine on x86.  
I use virtual studio 2017 command line. go to Detours\src. Only compile one 
```
SET DETOURS_TARGET_PROCESSOR=X64
nmake

SET DETOURS_TARGET_PROCESSOR=X86
nmake
```


### Hook API
 &emsp; &emsp;Create Device Symblink , Sony uses CreateFileA. So Only Hook One API is Fine.

### Command line  
this is Linux command line format. long is two "--"
```
--label=1
--hubname="USB#VID_2109&PID_2813#6&3183d08&0&1#{f18a0e88-c30c-11d0-8815-00a0c906bed8}"
--hubport=1
```
output:  
 &emsp; &emsp;Felicastatus=notfindfelica   
 &emsp; &emsp;Felicastatus=foundfelica


## Notice  
	// USB\VID_054C&PID_06C3\0513115 //this is s380 Not supported. It is deprecated produce.

