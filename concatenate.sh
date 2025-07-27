#!/bin/bash

# CoToMeter Project Concatenator - Unix/Linux/Mac Shell Script
# Usage: ./concatenate.sh [project_directory] [output_file]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Default values
PROJECT_DIR="${1:-.}"
OUTPUT_FILE="${2:-cotometer_project_concatenated.txt}"

echo -e "${PURPLE}🐱 CoToMeter Project Files Concatenator${NC}"
echo "====================================="
echo -e "${BLUE}Project Directory:${NC} $PROJECT_DIR"
echo -e "${BLUE}Output File:${NC} $OUTPUT_FILE"
echo

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    if ! command -v python &> /dev/null; then
        echo -e "${RED}❌ Error: Python is not installed or not in PATH${NC}"
        echo "Please install Python 3.x and try again"
        exit 1
    else
        PYTHON_CMD="python"
    fi
else
    PYTHON_CMD="python3"
fi

# Check Python version
PYTHON_VERSION=$($PYTHON_CMD --version 2>&1 | cut -d' ' -f2 | cut -d'.' -f1)
if [ "$PYTHON_VERSION" -lt 3 ]; then
    echo -e "${RED}❌ Error: Python 3.x is required${NC}"
    echo "Current Python version: $($PYTHON_CMD --version)"
    exit 1
fi

# Check if project directory exists
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Error: Project directory '$PROJECT_DIR' does not exist${NC}"
    exit 1
fi

# Create the Python script if it doesn't exist
SCRIPT_NAME="concatenate_project.py"
if [ ! -f "$SCRIPT_NAME" ]; then
    echo -e "${YELLOW}⚠️  Python script not found. Please ensure '$SCRIPT_NAME' is in the current directory${NC}"
    echo "You can download it from the artifacts above."
    exit 1
fi

# Run the Python concatenator script
echo -e "${BLUE}🔄 Running concatenation script...${NC}"
$PYTHON_CMD "$SCRIPT_NAME" "$PROJECT_DIR" "$OUTPUT_FILE"

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Error occurred during concatenation${NC}"
    exit 1
fi

echo
echo -e "${GREEN}✅ Concatenation completed successfully!${NC}"
echo -e "${BLUE}📄 Output file:${NC} $OUTPUT_FILE"
echo

# Display file statistics
if [ -f "$OUTPUT_FILE" ]; then
    FILE_SIZE=$(wc -c < "$OUTPUT_FILE" 2>/dev/null || stat -f%z "$OUTPUT_FILE" 2>/dev/null || echo "unknown")
    LINE_COUNT=$(wc -l < "$OUTPUT_FILE" 2>/dev/null || echo "unknown")
    
    echo -e "${BLUE}📊 File Statistics:${NC}"
    echo "   Size: $FILE_SIZE bytes"
    echo "   Lines: $LINE_COUNT"
    echo
fi

echo -e "${GREEN}💡 You can now upload this file to Claude for analysis${NC}"
echo -e "${GREEN}   or create a GitHub Gist with the content.${NC}"
echo

# Ask if user wants to open the output file (macOS only)
if [[ "$OSTYPE" == "darwin"* ]]; then
    read -p "Do you want to open the output file? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        open "$OUTPUT_FILE"
    fi
fi

echo -e "${GREEN}🎉 Done!${NC}"