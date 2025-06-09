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

echo "Запуск клиента для отображения информации..."
./client 127.0.0.1 9000 &
CLIENT_PID=$!

echo "Все процессы запущены. Для остановки нажмите Ctrl+C"
trap 'kill $SERVER_PID $CASHIER1_PID $CASHIER2_PID $GENERATOR_PID $CLIENT_PID; echo "Процессы остановлены"; exit' SIGINT

wait
