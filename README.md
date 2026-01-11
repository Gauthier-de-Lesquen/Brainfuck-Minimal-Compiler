# Brainfuck-Minimal-Compiler
*This is a minimal BrainFuck to C and then to executable compiler.* \
**⚠️ you need to have GCC installed on your computer ⚠️**

## How To Use It ?
To use it, you must do like when you use gcc. After compiling the C file (or downloading the .exe attached) and added it to path, you can use it like that : \
```bf code.bf```
> that will compile with the name code.exe



Else, you can compile like: \
```bf code.bf -o program.exe```
> that will produce a program.exe file



## In fact, What Is BrainFuck ?
> brainfuck is an esotheric/exotic language created by Urban Müller to be simple, but have an horrible syntax



Urban Müller imagined then a language running on a big table of unsigned characters (255 bytes) and that can be controled by only 8 commands:

|command| description                                                    |
|-------|----------------------------------------------------------------|
|```>```| increments the data pointer (position on the great table) of 1 |
|```<```| decrements the data pointer of 1                               |
|```+```| increments the byte at the data pointer by 1 modulo 256        |
|```-```| decrements the byte at the data pointer by 1 modulo 256        |
|```.```| displays the byte at the data pointer                          |
|```,```| accepts an input from the user at the data pointer             |
|```[```|	start of a loop while the data pointer isn't zero              |
|```]```| end of a [-starting loop                                       |

> FUN FACT: Brainfuck is turing-compete, that means you can run every programs you want 
