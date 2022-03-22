import csv
from progress.bar import IncrementalBar

rows_counter = 0  # Кол-во прочитанных строк из файла
data = {}  # Сюда читаем данные из файла
Sm = {}  # Выходной словать, где ключ-№ канала, а значение-список из Sm для каждой m[1, n/2]
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

for NumCanal in range(len(data)):
    # Инициализируем прогрессбар
    bar = IncrementalBar(f"Канал №{NumCanal+1}", max=int(rows_counter / 2))

    for m in range(1, int(rows_counter / 2) + 1):
        p = int(rows_counter / m)
        matrix = []  # Будущая таблица с m-столбцов и p-строк
        start = 0  # Начало среза
        average = []  # Сюда складывем усреднённые суммы элементов столбцов матрицы
        # Строим из списка матрицу
        for i in range(p):
            chunk = data[NumCanal][start:(start + m)]
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
        Sm[NumCanal].append((Smax - Smin) / 2 * p)
        bar.next()
    print("")

