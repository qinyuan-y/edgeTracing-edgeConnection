import csv

class ConfigParser:
    def load_config(filename):
        data = []

        with open(filename, 'r', encoding='UTF8', newline='') as f:
            reader = csv.reader(f, lineterminator='\n')
            for row in reader:
                data.append([i for i in row])

        return data

    def save_config(data, config_file):
        # write data into config file
        with open(config_file, 'w', encoding='UTF8', newline='') as f:
            writer = csv.writer(f, lineterminator='\n')

            # write the data
            writer.writerows(data)