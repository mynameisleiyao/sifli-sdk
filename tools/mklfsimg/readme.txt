Usage: mklfsimg -c <pack-dir> -b <block-size> -r <read-size> -p <prog-size> -s <filesystem-size> -i <image-file-path>
����size�����ֽ�Ϊ��λ

NAND�ļ�ϵͳ��mklfsimg -c test -b 131072 -r 2048 -p 2048 -s 20971520 -i test.bin
NOR�ļ�ϵͳ:  mklfsimg -c test -b 4096 -r 32 -p 256 -s 2097152 -i test.bin
