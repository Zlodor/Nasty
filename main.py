import csv

from progress.bar import IncrementalBar
import matplotlib.pyplot as plt
from threading import Thread
from multiprocessing import Process, Lock, Manager
from tqdm import tqdm
import time

rows_counter = 0  # Кол-во прочитанных строк из файла
data = {}  # Сюда читаем данные из файла
# Sm = {}  # Выходной словать, где ключ-№ канала, а значение-список из Sm для каждой m[1, n/2]
Sm = Manager().dict()
with open('01_gauss8_0grad.csv', 'r', newline='', encoding='utf-8') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    tmp = next(reader)
    num_canals = len(tmp)
    for i in range(num_canals):
        data[i] = [float(tmp[i])]
        Sm[i] = []
    print(f"Количество каналов - {num_canals}")
    for row in reader:
        for i in range(num_canals):
            data[i].append(float(row[i]))
    rows_counter = len(data[0])
    print(f"Строк прочитано - {rows_counter}")


# Однопоточная обработка
# for NumCanal in range(len(data)):
#     # Инициализируем прогрессбар
#     bar = IncrementalBar(f"Канал №{NumCanal+1}", max=int(rows_counter / 2))
#
#     for m in range(1, int(rows_counter / 2) + 1):
#         p = int(rows_counter / m)
#         matrix = []  # Будущая таблица с m-столбцов и p-строк
#         start = 0  # Начало среза
#         average = []  # Сюда складывем усреднённые суммы элементов столбцов матрицы
#         # Строим из списка матрицу
#         for i in range(p):
#             chunk = data[NumCanal][start:(start + m)]
#             start += m
#             matrix.append(chunk)
#         # Считаем среднее значение столбцов
#         for i in range(m):
#             summ = 0
#             for chunk in matrix:
#                 summ += chunk[i]
#             average.append(summ / p)
#
#         Smin = min(average)
#         Smax = max(average)
#         Sm[NumCanal].append((Smax - Smin) / (2 * p))
#         bar.next()
#     print("")   # Нужно чтобы следующий бар рисовался на новой строке


# Многопоточная обработка
# def sm_calculate(num_canal):
#     for m in tqdm(range(1, int(rows_counter / 2) + 1)):
#         p = int(rows_counter / m)
#         matrix = []  # Будущая таблица с m-столбцов и p-строк
#         start = 0  # Начало среза
#         average = []  # Сюда складывем усреднённые суммы элементов столбцов матрицы
#         # Строим из списка матрицу
#         for i in range(p):
#             chunk = data[num_canal][start:(start + m)]
#             start += m
#             matrix.append(chunk)
#         # Считаем среднее значение столбцов
#         for i in range(m):
#             summ = 0
#             for chunk in matrix:
#                 summ += chunk[i]
#             average.append(summ / p)
#
#         Smin = min(average)
#         Smax = max(average)
#         Sm[num_canal].append((Smax - Smin) / (2 * p))
#
#
# # Запускаем потоки и ожидаем их завершения
# th_list = []
# for num in range(len(data)):
#     th = Thread(target=sm_calculate, args=(num,))
#     th_list.append(th)
#     th.start()
# for th in th_list:
#     th.join()


# Многопроцессовая обработка
lock = Lock()


def sm_calculate_process(canal_data: list, num_canal: int, lock_copy):
    """
    Функцйия для вызова в многопроцессном режиме
    :param lock_copy: копия объекта типа Lock, для синхронизации процессов.
    :param canal_data: информация с канала в сыром виде
    :param num_canal: номер канала (нумерация начинается с 0)
    :return: void
    """
    global Sm
    sm_local = []
    for m in tqdm(range(1, int(rows_counter / 2) + 1)):
        p = int(rows_counter / m)
        matrix = []  # Будущая таблица с m-столбцов и p-строк
        start = 0  # Начало среза
        average = []  # Сюда складывем усреднённые суммы элементов столбцов матрицы
        # Строим из списка матрицу
        for i in range(p):
            chunk = canal_data[start:(start + m)]
            start += m
            matrix.append(chunk)
        # Считаем среднее значение столбцов
        for i in range(m):
            summ = 0
            for chunk in matrix:
                summ += chunk[i]
            average.append(summ / p)

        Smin = min(average)
        Smax = max(average)
        sm_local.append((Smax - Smin) / (2 * p))

    lock_copy.acquire()
    Sm[num_canal] = sm_local
    lock_copy.release()


# Запускаем процессы и ожидаем их завершения
pr_list = []
for num in range(len(data)):
    pr = Process(target=sm_calculate_process, args=(data[num], num, lock, ))
    pr_list.append(pr)
    pr.start()
for pr in pr_list:
    pr.join()


def save_results(name_output_file='output.csv'):
    with open(name_output_file, 'w+', newline='', encoding='utf-8') as ouput_file:
        print("Запись результатов в файл:")
        writer = csv.writer(ouput_file, delimiter=',')
        for i in tqdm(range(len(Sm[0]))):
            line = []
            for j in range(len(Sm)):
                line.append(Sm[j][i])
            writer.writerow(line)


th = Thread(target=save_results, args=())
th.start()
# Вычислим кол-во ячеек под графики взависимости от числа входных каналов
n = 1
m = 1
while n * m < len(data):
    n += 1
    if n * m >= len(data):
        break
    else:
        m += 1
# Инициализируем графики
for i in range(len(data)):
    plt.subplot(n, m, i + 1)
    plt.plot(Sm[i])
    plt.title(f"Канал №{i + 1}")
plt.show()
th.join()
