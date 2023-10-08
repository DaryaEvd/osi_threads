## Полезные ссылки  
- [Cтатья на хабре про многопоочность](https://habr.com/ru/companies/otus/articles/549814/)  
- [Здесь](https://dmilvdv.narod.ru/Translate/ELSDD/index.html) смотреть главу [прграммирвание с помощью pthread-ов](https://dmilvdv.narod.ru/Translate/ELSDD/elsdd_programming_with_pthreads.html)  
- [САЙТ-НАХОДКА](https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/index.html) - в частности, смотреть [главу 6](https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/ThreadsOverview.html)  
- [Чёт с итмо](https://se.ifmo.ru/~ad/Education_Information/System_Soft/Mod_8/Unix/Part3/preparing3.html)  
- [Какое-то уч пособие](https://portal.tpu.ru/SHARED/v/VSS/Study_work/OS/Tab/lab3.pdf)  


### Откопать потом   
Спеку для pthreads - [вопросик](https://stackoverflow.com/questions/9625036/what-is-the-official-standard-for-pthreads)  


### Для 1.1  
#### 1.1
  -  a) Оба потока не выполнялись одновременно, потому что главный main() завершался быстрее.  
  -  c) Используем функцию pthread_equal, потому что 
  "the definition of pthread_t is implementation dependent", 
  "It might be a struct. It might be a pointer. It might be a pointer to a struct held somewhere"  
  Смотреть [тут](https://stackoverflow.com/questions/37675763/why-cannot-i-directly-compare-2-thread-ids-instead-of-using-pthread-equal)   
  - e) смотреть [сюда]( 
  https://github.com/DaryaEvd/osi_threads/blob/44b7c8268304573f27fe8abbce7a8bacbfc4483d/lab1/code/1.1/theo.txt)   


### Атрибуты потоков  

#### Отсоединенное состояние:  
`int pthread_attr_setdetachstate(pthread+attr_t *attr, detachstate)`  

- detached - основной поток может не дожидаться завершения дочернего потока  
- joinable - основной поток должен дождаться завершения дочернего потока  
  
### Про joinable  
Можно почитать [тут](https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-pthread-join-wait-thread-end), [ин рашн](https://www.ibm.com/docs/ru/aix/7.2?topic=programming-joining-threads), [тут](https://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_join.html#:~:text=The%20pthread_join()%20function%20provides,were%20used%20by%20the%20thread.)  

### Про detached  
Читаем вот [здесь](https://dmilvdv.narod.ru/Translate/ELSDD/elsdd_detached_threads.html)  

### !  
То есть когда мы испоьзуем join, то это гарантирует, что к тому моменту, как завершится вызывающий поток (main), created thread закончит своё существование.  

Когда мы используем detached thread, то это значит, что когда вызывающий поток (main) завершится, то созданный тред - нет, он прдолжит своё существование.  

**According to the pthread specification, all threads are supposed to be created as joinable by default!!!**  
(взято [отсюда](https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/POSIXThreads.html))  


### Про бесконечный цикл   
При запуске `./d.o` будет такой вывод:  
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
При запуске `./e.o` треды будут создаваться бесконечно, т.к. detached thread will automatically be disposed when it ends execution.   

Почитать можно [тут](https://stackoverflow.com/questions/42442990/pthread-create-cannot-allocate-memory)  


### Про cancel 
Украдено [отсюда](https://habr.com/ru/articles/326138/).  

Точно так же, как при управлении процессами, иногда необходимо досрочно завершить процесс, многопоточной программе может понадобиться досрочно завершить один из потоков. Для досрочного завершения потока можно воспользоваться функцией pthread_cancel.  

Важно понимать, что несмотря на то, что pthread_cancel() возвращается сразу и может завершить поток досрочно, ее нельзя назвать средством принудительного завершения потоков. Дело в том, что поток не только может самостоятельно выбрать момент завершения в ответ на вызов pthread_cancel(), но и вовсе его игнорировать. Вызов функции pthread_cancel() следует рассматривать как запрос на выполнение досрочного завершения потока. Поэтому, если для вас важно, чтобы поток был удален, нужно дождаться его завершения функцией pthread_join().  

[Здесь](https://se.ifmo.ru/~ad/Education_Information/System_Soft/Mod_8/Unix/Part3/preparing3.html) читать с "Досрочное завершение потока", откуда ~~украдено~~ (взято) следующее: 

Интересна роль функции pthread_testcancel(). Как уже отмечалось, эта функция создает точку отмены потока. Зачем нужны особые точки отмены? Дело в том, что даже если досрочное завершение разрешено, поток, получивший запрос на досрочное завершение, может завершить работу не сразу. Если поток находится в режиме отложенного досрочного завершения (именно этот режим установлен по умолчанию), он выполнит запрос на досрочное завершение, только достигнув одной из точек отмены. В соответствии со стандартом POSIX, точками отмены являются вызовы многих ?обычных? функций, например open(), pause() и write(). Про функцию printf() в документации сказано, что она может быть точкой отмены, но в ОС Linux при попытке остановиться на printf() происходит нечто странное ? поток завершается, но pthread_join() не возвращает управления. Поэтому мы создаем явную точку отмены с помощью вызова pthread_testcancel().  

### про cancellation and printf  
- Посмотреть осуждение на [stackoverflow](https://stackoverflow.com/questions/17208690/cancelling-a-pthread-and-printf-behaviour)  
- почитать про [cancellation point](https://stackoverflow.com/questions/27374707/what-exactly-is-a-cancellation-point#comment43199960_27374707)(!!!).  
- еще одно обсуждение на [оверфлоу](https://stackoverflow.com/questions/23220206/what-are-pthread-cancelation-points-used-for) + там про `PTHREAD_CANCEL_ASYNCHRONOUS` есть  

#### Важно  
When your thread gets pulled from execution, its state is saved by the OS and that is not a cancellation of the thread. The cancellation means thread termination, on request, with the specific intent of letting everything in a final state when completed (aka. all resources are freed, all handlers are updated, etc.).

What you call blocking can happen to a thread while in mid-cancellation.

Example: The thread gets a cancellation request. The OS queues it until the thread becomes cancellable. When the thread becomes cancellable, and the thread is executing a cancel point, the thread can be cleaned and cancelled. The write function is a cancellation point, this meaning it is safe from the point of view of the OS to cancel the thread while this function is executed (the state of all related resources will be consistent).

While the cancellation procedure is running, the thread can be blocked as many times as the OS sees fit.  

`PTHREAD_CANCEL_ASYNCHRONOUS` is another ball of wax and nightmare of its own. Conceptually you can almost think of this as the thread getting blasted by a kill signal - it can be interrupted anywhere - but it does provide the opportunity for the cleanup handlers to run. The problem is that it is really hard to do that e.g. what if it is canceled in the middle of a malloc? This makes it pretty much useless outside of some really, really, carefully considered situations.

Посмотреть [ответ](https://stackoverflow.com/questions/74100179/usage-of-pthread-cleanup-push-and-pthread-cleanup-pop) про `pthread_cleanup_push` and `pthread_cleanup_pop`.  
