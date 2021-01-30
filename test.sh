gcc main.c -o ivm-hex-dump

# Sample data from implementation guide:
#
echo "200 e15 2406097920102c06097420102c06096f" > input-1.txt
echo "201 38a 2406097920102c06097420102c06096f" > input-2.txt
echo "200 da4 2406097920102c06097420102c06096e" > input-3.txt

echo '2406097920102c06097420102c06096f' | xxd -r -p > input-1.bin
echo '2406097920102c06097420102c06096f' | xxd -r -p > input-2.bin
echo '2406097920102c06097420102c06096e' | xxd -r -p > input-3.bin

./ivm-hex-dump --start-row `printf '%d' 0x200` input-1.bin
cmp 00000.txt input-1.txt
./ivm-hex-dump --start-row `printf '%d' 0x200` --decode output-1.bin
cmp input-1.bin output-1.bin

./ivm-hex-dump --start-row `printf '%d' 0x201` input-2.bin
cmp 00000.txt input-2.txt
./ivm-hex-dump --start-row `printf '%d' 0x201` --decode output-2.bin
cmp input-2.bin output-2.bin

./ivm-hex-dump --start-row `printf '%d' 0x200` input-3.bin
cmp 00000.txt input-3.txt
./ivm-hex-dump --start-row `printf '%d' 0x200` --decode output-3.bin
cmp input-3.bin output-3.bin

