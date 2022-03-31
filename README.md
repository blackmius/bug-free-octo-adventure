# Домашнее задание по РОС

```
./ping ADDR [LOGFILE]
```

# Как собрать

```
cmake -B build -G "Unix Makefiles"
cmake --build build
# Появился файл
# ./build/ping
```


# Как запустить без привилегий суперпользователя
```
sudo setcap cap_net_raw=eip build/ping
```