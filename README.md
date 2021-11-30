# sistemasEmbarcados
Laboratórios da disciplina de sistemas embarcados - 2021/2

## LAB 1
O resultado do arquivo main.cpp aparece no terminal I/O, visível durante o debug do código

## LAB 2
A variável ui32Loop é declarada como volatile dentro da main porque ela pode ser alterada por eventos externos

## LAB 5
Tabelas:
![alt text](Lab5/tabelasImages.png)
Diagrama:
![alt text](Lab5/diagramaThreadX.png)

## LAB 6 
Tempo de execução de cada loop: 1.39 us 

Questão a) Escalonamento por time-slice de 50 ms. Todas as tarefas com mesma prioridade.
R:Neste modo, os leds piscam de forma alternada com time-slice de 50 ms.

Questão b) Escalonamento sem time-slice e sem preempção. Prioridades estabelecidas no passo 3. A preempção pode ser evitada com o “preemption threshold” do ThreadX.
R:As tarefas precisam esperar a tarefa que está sendo executada para que o processador seja liberado. Quando ele estiver liberado, as tarefas serão executadas conforme suas prioridades (de forma prática: os leds ficam piscando alternadamente com um tempo de intervalo praticamente constante)

Questão c) Escalonamento preemptivo por prioridade.
R:O led com maior prioridade sempre interrompe a execução de qualquer tarefa corrente. Quando a tarefa é interrompida, ela aguarda até poder voltar a executar, o que pode tornar seu tempo de execução bem maior do que o previsto. 

Questão d) Implemente um Mutex compartilhado entre T1 e T3. No início de cada job estas tarefas devem solicitar este mutex e liberá-lo no final. Use mutex sem herança de prioridade. Observe o efeito na temporização das tarefas.
R:Ao solicitar o mutex, as threads 1 e 3 não podem ativar a preempção entre si, causando assim uma variação nos períodos de execução de cada thread. No caso da thread 2, seu tempo de execução pode aumentar caso a thread 1 altere do status pending para ready; já a thread 3 só terá seu tempo incrementado caso a thread 1 esteja utlizando o mutex ou a thread 2 esteja pronta para executar.

Questão e) Idem acima, mas com herança de prioridade.
R:Com herança de prioridade do mutex, a principal diferença será no caso da thread 2 que terá seu tempo de execução incrementado caso a thread 3 esteja em execução.