Testcase found by FirmSmith

Fails on call "cparser ./bugreports/a_-funroll-loops/20180708171533-3.ir -O0 --target=x86_64-linux-gnu -funroll-loops" w/ Segemntation fault (no further error message)
input is compilable when using optimization level 1

--> Try to reduce by using compilable script: Reduction + result
--> error msg changes, now using a script that greps "Segmentation fault"
--> not reducible at all
