# Блок задач на синхронизацию  

### Для 2.1  

#### Пункт a
Вспомнить, что такое очередь и [как она работает](https://codelessons.dev/ru/ochered-queue-v-c-realizaciya-i-chto-eto-voobshhe-takoe/).   
На примере [стека](https://prepinsta.com/c-program/implementation-of-queues-using-linked-list/)  

#### Пункт b
При изначальном варианте пограммы `./queue-threads` возникали следующие ошибки:  
`ERROR: get value is 4372464 but expected - 4372463
Segmentation fault (core dumped)`  
или просто 
`Segmentation fault (core dumped)`  

**Полезно!** : [Как смотреть core files?](https://unix.stackexchange.com/questions/89933/how-to-view-core-files-for-debugging-purposes-in-linux)  
Или так: 
`./queue-threads`   
`coredumpctl gdb -1`  
В открывшемся окне видим:  
`Storage: /var/lib/systemd/coredump/core.queue-threads.1000.4808e08477e8414eb47fdb2179cb815e.45697.1698337778000000000000.lz4 (inaccessible)`  
Копируем `/var/lib/systemd/coredump/core.queue-threads.1000.4808e08477e8414eb47fdb2179cb815e.45697.1698337778000000000000.lz4 `  
Затем для удобства пишем в терминальчик (например так):  
`sudo cp  /var/lib/systemd/coredump/core.queue-threads.1000.4808e08477e8414eb47fdb2179cb815e.45697.1698337778000000000000.lz4  ./core`  
`sudo chmod ugo+rwx core`  
`mv ./core core.lz4`  
`unlz4 core.lz4`  
`gdb ./core queue-threads`  
Потом жмякаем на enter и пишем `where`

Получим что-то в таком духе:  
```
Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x00005605e73446a1 in queue_get (q=0x5605e81196b0, 
    val=0x7eff212f2ec4) at queue.c:102
102       *val = tmp->val;           // take val of the 1st node
[Current thread is 1 (Thread 0x7eff212f3700 (LWP 45699))]
(gdb) where
#0  0x00005605e73446a1 in queue_get (q=0x5605e81196b0, 
    val=0x7eff212f2ec4) at queue.c:102
#1  0x00005605e7344949 in reader (arg=0x5605e81196b0)
    at queue-threads.c:45
#2  0x00007eff21cf2609 in start_thread (arg=<optimized out>)
    at pthread_create.c:477
#3  0x00007eff21c17133 in clone ()
    at ../sysdeps/unix/sysv/linux/x86_64/clone.S:95
```  
Пытались забрать `tmp`. Посмотрим, что это такое.   
Пишем дальше:  
`p tmp`  
Получаем вот что:  
`$1 = (qnode_t *) 0x0`  
То есть мы пытались разыменовать нулевой указатель.  
Можем еще посмотреть соседние потоки. Пишем:  
`info thread`  
Получаем:  
```
  Id   Target Id                                   Frame 
* 1    Thread 0x7eff212f3700 (LWP 45699)           0x00005605e73446a1 in queue_get (q=0x5605e81196b0, val=0x7eff212f2ec4) at queue.c:102
  2    Thread 0x7eff20af2700 (LWP 45700)           0x00007eff21b92190 in checked_request2size (sz=<optimized out>, req=<optimized out>)
    at malloc.c:3059
  3    Thread 0x7eff21af4700 (LWP 45698)           0x00007eff21bd523f in __GI___clock_nanosleep (clock_id=clock_id@entry=0, 
    flags=flags@entry=0, req=req@entry=0x7eff21af3e80, 
--Type <RET> for more, q to quit, c to continue without paging--
    eff21af3e80) at ../sysdeps/unix/sysv/linux/clock_nanosleep.c:78
  4    Thread 0x7eff21af5740 (LWP 45697) (Exiting) warning: Couldn't find general-purpose registers in core file.
<unavailable> in ?? ()
```  
Наш поток упал в `queue_get`, а соседний(thread 2) - был в `checked_request2size` (не оч пон чо ито)  
Можем написать для более подробной инфы:  
`thread 2`  
`where`  
Чтоб выйти из gdb пишем `quit`  

То есть у нас есть `tmp=0`. Вопрос: почему?  
Потому что поток что-то успелось сделаться, а что-то нет. Эти неатомарные вещи в конкурентной среде происходят несинхронизированно. 


### Теория про планирование и shed_yield() в частности
- [Про планирование в Lunux](https://habr.com/ru/companies/ruvds/articles/578788/)  
- Статья про shed_yield() [number1](https://it.wikireading.ru/1764)  
- [number2](https://it.wikireading.ru/1764)  
- [number3](https://www.halolinux.us/kernel-reference/the-sched-yield-system-call.html)  


From the 2nd article украдено:  
```
Состояние «выполняется»

Как только планировщик поставил процесс на выполнение, началось состояние «выполняется». Процесс может выполняться весь предложенный промежуток (квант) времени, а может уступить место другим процессам, воспользовавшись системным вывозом sched_yield.
```


### Доп инфа  
- [Про cas операции](https://en.wikipedia.org/wiki/Compare-and-swap#:~:text=In%20computer%20science%2C%20compare%2Dand,to%20a%20new%20given%20value.)  
