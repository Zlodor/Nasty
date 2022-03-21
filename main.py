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

# for m in range(1, int(rows_counter/2)):


