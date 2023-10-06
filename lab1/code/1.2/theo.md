## Полезные ссылки  
- [статья на хабре про многопоочность](https://habr.com/ru/companies/otus/articles/549814/)  


### Атрибуты потоков  

#### Отсоединенное состояние:  
`int pthread_attr_setdetachstate(pthread+attr_t *attr, detachstate)`  

- detached - основной поток может не дожидаться завершения дочернего потока  
- joinable - основной поток должен дождаться завершения дочернего потока  
  
### Про joinable  
Можно почитать [тут](https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-pthread-join-wait-thread-end), [ин рашн](https://www.ibm.com/docs/ru/aix/7.2?topic=programming-joining-threads), [тут](https://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_join.html#:~:text=The%20pthread_join()%20function%20provides,were%20used%20by%20the%20thread.)  
