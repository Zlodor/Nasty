import csv

rows_counter = 0
data = {}
with open('01_gauss8_0grad.csv', 'r', newline='', encoding='utf-8') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    tmp = next(reader)
    num_canals = len(tmp)
    for i in range(num_canals):
        data[i] = [tmp[i]]
    print(f"Количество каналов - {num_canals}")
    for row in reader:
        for i in range(num_canals):
            data[i].append(row[i])
    rows_counter = len(data[0])
    print(f"Строк прочитано - {rows_counter}")

for m in range(1, int(rows_counter/2) + 1):
    p = int(rows_counter / m)
    matrix = []  # Будущая таблица с m столбцов
    start = 0  # Начало среза
    for i in range(p):
        chunk = data[0][start:(start + m)]
        start += m
        matrix.append(chunk)
    # print(f"{m}) p:{p}/{len(matrix)}")
