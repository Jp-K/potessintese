import csv 


with open('FlowerDatabase.csv', 'r', encoding='utf-8') as file:
    csv_reader = csv.DictReader(file)
    data = []
    for row in csv_reader:
        data.append(row)
    

for row in data:
    print(row['Name'])