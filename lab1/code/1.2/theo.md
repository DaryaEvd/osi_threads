## Полезные ссылки  
- [статья на хабре про многопоочность](https://habr.com/ru/companies/otus/articles/549814/)  


### Атрибуты потоков  

#### Отсоединенное состояние:  
`int pthread_attr_setdetachstate(pthread+attr_t *attr, detachstate)`  

- detached - основной поток может не дожидаться завершения дочернего потока  
- joinable - основной поток должен дождаться завершения дочернего потока  
  
### Про joinable  
Можно почитать [тут](https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-pthread-join-wait-thread-end), [ин рашн](https://www.ibm.com/docs/ru/aix/7.2?topic=programming-joining-threads), [тут](https://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_join.html#:~:text=The%20pthread_join()%20function%20provides,were%20used%20by%20the%20thread.)  


### Про бесконечный цикл   
Призапуске `./d` будет такой вывод:  
```
...
tID: 140048272058112
tID: 140048263665408
tID: 140048255272704
tID: 140048246880000
tID: 140048238487296
pthread_create() error: Cannot allocate memory
```  
А почему? Потому что виртуальная память для выделения стека для очередного треда заканчивается + треды не осовбождают память после своего завершения.  

#### Что изменится, если добавить detached?  
При запуске `./e` треды будут создаваться бесконечно, т.к. detached thread will automatically be disposed when it ends execution.   

Почитать можно [тут](https://stackoverflow.com/questions/42442990/pthread-create-cannot-allocate-memory)  


