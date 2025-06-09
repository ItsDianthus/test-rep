#!/bin/bash

gcc -o server server.c
gcc -o generator generator.c
gcc -o cashier cashier.c
gcc -o client client.c

echo "Запуск сервера..."
./server 9000 127.0.0.1 9001 127.0.0.1 9002 5 &
SERVER_PID=$!

echo "Запуск кассиров..."
./cashier 9001 127.0.0.1 9000 2000 &
CASHIER1_PID=$!
./cashier 9002 127.0.0.1 9000 3000 &
CASHIER2_PID=$!

echo "Запуск генератора покупателей..."
./generator 127.0.0.1 9000 1000 &
GENERATOR_PID=$!

CLIENT_PIDS=()

function start_client {
    echo "Запуск клиента для отображения информации..."
    ./client 127.0.0.1 9000 &
    CLIENT_PIDS+=($!)
}

start_client

while true; do
    echo -n "Введите команду (start - запуск клиента, stop - остановка клиента, quit - завершение работы): "
    read command

    if [[ "$command" == "start" ]]; then
        start_client
    elif [[ "$command" == "stop" ]]; then
        if [[ ${#CLIENT_PIDS[@]} -gt 0 ]]; then
            # Остановка последнего запущенного клиента
            last_client_pid=${CLIENT_PIDS[-1]}
            kill $last_client_pid
            echo "Остановлен клиент с PID $last_client_pid"
            # Удаляем PID клиента из массива
            unset CLIENT_PIDS[-1]
        else
            echo "Нет клиентов для остановки."
        fi
    elif [[ "$command" == "quit" ]]; then
        break
    else
        echo "Неизвестная команда. Пожалуйста, используйте 'start', 'stop' или 'quit'."
    fi
done

trap 'kill $SERVER_PID $CASHIER1_PID $CASHIER2_PID $GENERATOR_PID ${CLIENT_PIDS[@]}; echo "Процессы остановлены"; exit' SIGINT

wait