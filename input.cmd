gzip < testcase/0.txt > testcase/0_temp.gz 
gzip < testcase/1.txt > testcase/1_temp.gz
gzip < testcase/2.txt > testcase/2_temp.gz
gzip < testcase/3.txt > testcase/3_temp.gz
gzip < testcase/4.txt > testcase/4_temp.gz
gzip < testcase/5.txt > testcase/5_temp.gz
gzip < testcase/6.txt > testcase/6_temp.gz
gzip < testcase/7.txt > testcase/7_temp.gz
#sync
gunzip < testcase/0_temp.gz > testcase/0_output.txt
gunzip < testcase/1_temp.gz > testcase/1_output.txt
gunzip < testcase/2_temp.gz > testcase/2_output.txt
gunzip < testcase/3_temp.gz > testcase/3_output.txt
gunzip < testcase/4_temp.gz > testcase/4_output.txt
gunzip < testcase/5_temp.gz > testcase/5_output.txt
gunzip < testcase/6_temp.gz > testcase/6_output.txt
gunzip < testcase/7_temp.gz > testcase/7_output.txt
#sync
rm testcase/0_temp.gz 
rm testcase/1_temp.gz
rm testcase/2_temp.gz
rm testcase/3_temp.gz
rm testcase/4_temp.gz
rm testcase/5_temp.gz
rm testcase/6_temp.gz
rm testcase/7_temp.gz
#exit

