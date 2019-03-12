import re
import os




fileName = "Kyooshi Gothic.ttf"
#fileName = "font_test.bin"
fileName = "src/" + fileName

numBytes = os.path.getsize(fileName)
output = open("src/font_header.h", "w")

output.write("// This file was generated by generate_font_header.py\n")
output.write("\n")
output.write("#define FONT_DATA_SIZE " + str(numBytes) + "\n")
output.write("FT_Byte fontData[FONT_DATA_SIZE] = {\n        ")

lineLength = 0    
with open(fileName, "rb") as f:
    byte = f.read(1)
    while byte != b"":
        numBytes -= 1
        h = str(hex(int.from_bytes(byte, 'little')))[2:]
        h = "0" + h
        h = "0x" + h[-2:]
        
        output.write(h)
        #print (h)
        byte = f.read(1)

        if numBytes == 0:
            output.write("};\n")
        else:
            output.write(", ")
        
        lineLength += 1
        if lineLength >= 16:
            lineLength = 0;
            output.write("\n        ")
            
    #output.write(s)
    #print("Opening " + "res/" + fileName)
    #glFile = open("res/" + fileName, "r")
    #for line in glFile:
    #    output.write('\n    "' + line[0:-1] + '\\n"')
    #glFile.close()
    #
    #output.write("};\n\n")
    #print (s)


output.close()
