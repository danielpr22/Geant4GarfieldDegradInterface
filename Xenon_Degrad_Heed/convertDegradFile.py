#!/usr/bin/env python
#!/usr/bin/env python3
import os

# Get the current working directory
WORKING_DIRECTORY = os.getcwd()
print(f"Working directory: {WORKING_DIRECTORY}")

# Process all .OUT files in the working directory
for file_name in os.listdir(WORKING_DIRECTORY):
    if file_name.endswith(".OUT"):
        print(f"Processing file: {file_name}")

        # Replace all spaces with tabs
        temp_file = "tempFile.dat"
        replacements = {' ': '\t'}
        try:
            with open(file_name, "r") as infile, open(temp_file, "w") as outfile:
                print("Replacing spaces with tabs...")
                for line in infile:
                    for src, target in replacements.items():
                        line = line.replace(src, target)
                    outfile.write(line)

            # Replace all double tabs with single tabs
            with open(temp_file, "r") as infile, open(file_name, "w") as outfile:
                print("Replacing double tabs with single tabs...")
                for line in infile:
                    while '\t\t' in line:
                        line = line.replace('\t\t', '\t')
                    outfile.write(line)

            # Clean up temporary file
            os.remove(temp_file)
            print(f"Finished processing file: {file_name}")

        except Exception as e:
            print(f"Error processing file {file_name}: {e}")

print("All files processed successfully.")