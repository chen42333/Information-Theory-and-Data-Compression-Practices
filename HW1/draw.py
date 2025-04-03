import matplotlib.pyplot as plt
import os
import sys

def read_pmf_file(file_path):
    tables = []
    current_title = None
    current_data = []

    with open(file_path, 'r') as f:
        lines = f.readlines()

    for line in lines:
        line = line.strip()
        if line.startswith('-----'):
            if current_title and current_data:
                tables.append((current_title, current_data))
            current_title = line.strip('-').strip()  # Get title
            current_data = []
        elif line:
            symbol, probability = line.split()
            current_data.append((symbol, float(probability)))

    if current_title and current_data:
        tables.append((current_title, current_data))

    return tables

def plot_pmf(title, pmf_data, output_folder):
    symbols = [symbol for symbol, _ in pmf_data]
    probabilities = [prob for _, prob in pmf_data]

    plt.bar(symbols, probabilities)
    plt.xlabel('Symbols')
    plt.ylabel('Probability')
    plt.title(title)

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    file_name = f"{title}.png"
    file_path = os.path.join(output_folder, file_name)
    plt.savefig(file_path)
    plt.close()

file_path = sys.argv[1]
output_folder = sys.argv[2] 

tables = read_pmf_file(file_path)

for title, pmf_data in tables:
    plot_pmf(title, pmf_data, output_folder)