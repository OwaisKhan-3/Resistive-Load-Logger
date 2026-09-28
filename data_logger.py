import serial
import csv

adc_conversion = 4.39 / 1023
headers = ["Time (ms)", "Alkaline", "Zinc"]
connection = serial.Serial("COM3", 9600, timeout=1)

print("Serial communication started... ")

with open("resistive_load.csv", "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(headers)
    file.flush()

    while True:
        raw_data: bytes = connection.readline()
        print("Received raw data: ", raw_data)

        if raw_data:
            decoded_data = raw_data.decode("utf-8").strip()
            if decoded_data == "DONE":
                print("Data collection completed. Exiting...")
                break
            refined_data: list[str] = decoded_data.split(",")
            time_ms: int = int(refined_data[0])
            data_points: list[float] = [float(value) for value in refined_data[1:]]
            data_points = [(value * adc_conversion) for value in data_points]
            print("Writing data to CSV: ", [time_ms] + data_points)
            writer.writerow([time_ms] + data_points)
            file.flush()



    
