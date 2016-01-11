gcc -Wall myfs.c myfs_api.c `pkg-config fuse --cflags --libs`
sudo ./a.out -d vlm