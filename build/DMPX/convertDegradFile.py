#!/usr/bin/env python3
import os

# Get the current working directory
WORKING_DIRECTORY = os.getcwd()
print(f"(Debug: convertDegradFile.py) Working directory: {WORKING_DIRECTORY}")

# Process all .OUT files in the working directory
for file_name in os.listdir(WORKING_DIRECTORY):
    if file_name.endswith(".OUT"):
        print(f"(Debug: convertDegradFile.py) Processing file: {file_name}")

        # Replace all spaces with tabs
        temp_file = "tempFile.dat"
        replacements = {' ': '\t'}
        try:
            with open(file_name, "r") as infile, open(temp_file, "w") as outfile:
                print("(Debug: convertDegradFile.py) Replacing spaces with tabs...")
                for line in infile:
                    for src, target in replacements.items():
                        line = line.replace(src, target)
                    outfile.write(line)

            # Replace all double tabs with single tabs
            with open(temp_file, "r") as infile, open(file_name, "w") as outfile:
                print("(Debug: convertDegradFile.py) Replacing double tabs with single tabs...")
                for line in infile:
                    while '\t\t' in line:
                        line = line.replace('\t\t', '\t')
                    outfile.write(line)

            # Clean up temporary file
            os.remove(temp_file)
            print(f"(Debug: convertDegradFile.py) Finished processing file: {file_name}")

        except Exception as e:
            print(f"(Debug: convertDegradFile.py) Error processing file {file_name}: {e}")

print("(Debug: convertDegradFile.py) All files processed successfully.")