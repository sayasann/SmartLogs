Smart Home Log Conversion Tool

This C-based command-line tool is designed to convert smart home device logs between CSV, Binary, and XML formats. It also includes XSD validation for the generated XML files.
📌 Features

    Convert logs from CSV to Binary (logdata.dat)

    Convert logs from Binary to XML, using configurable sorting rules from a JSON file

    Endian conversion for integer fields with hex representation

    Generate XSD schema to validate the XML structure and content

    Handle various delimiters and operating system line endings

    Command-line arguments and help support

📁 Input Format

Input CSV file: smartlog.csv

Each row must include the following fields:
Field	Description
device_id	Format: 3 uppercase letters + 4 digits (e.g. THM1234). Required
timestamp	ISO 8601 format (e.g. 2025-03-01T14:25:00)
temperature	Float, range: -30.0 to 60.0
humidity	Integer, range: 0–100
status	One of: ! (ON), " (OFF), ⚠ (ERROR)
location	Max 30 characters
alert_level	One of: LOW, MEDIUM, HIGH, CRITICAL
battery	Integer 0–100. Required
firmware_ver	Format: vX.Y.Z. Range must be explicitly defined
event_code	Integer 0–255


🛠 Usage

./your_program -separator <1|2|3> -opsys <1|2|3> [-h]

    -separator: 1 = comma ,  2 = tab \t  3 = semicolon ;

    -opsys: 1 = Windows (\r\n), 2 = Linux (\n), 3 = macOS (\r)

    -h: show help/usage info

⚙️ Components
1. CSV to Binary

Converts smartlog.csv to logdata.dat, considering delimiter and OS line endings.
2. Binary to XML

    Reads sorting config from setupParams.json

    Sorts entries by key in ASC/DESC order

    Generates XML with root name matching output filename

    event_code includes Big Endian and Little Endian hex, plus decimal from hexLittle

3. XSD Schema Validation

Validates the generated XML file:

    Pattern checks (e.g., device_id, firmware_ver)

    Required fields

    Range and enum restrictions

🔗 File Naming Conventions

    Source code: <your_id>.c

    Schema: <your_id>.xsd

    Output binary: logdata.dat

    Output XML: <your_output_name>.xml
