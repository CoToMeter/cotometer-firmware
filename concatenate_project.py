#!/usr/bin/env python3
"""
CoToMeter Project Files Concatenator
Combines all project files into a single file for easy sharing
"""

import os
import sys
from pathlib import Path
from datetime import datetime

def should_include_file(file_path):
    """
    Determine if a file should be included in the concatenation
    """
    # File extensions to include
    include_extensions = {
        '.cpp', '.c', '.hpp', '.h', '.ino', 
        '.ini', '.txt', '.md', '.json', '.xml',
        '.yaml', '.yml', '.conf', '.cfg'
    }
    
    # Files to always include
    always_include = {
        'platformio.ini', 'CMakeLists.txt', 'Makefile', 
        'README.md', 'LICENSE', '.gitignore'
    }
    
    filename = file_path.name.lower()
    extension = file_path.suffix.lower()
    
    # Skip hidden files and directories (except specific ones)
    if filename.startswith('.') and filename not in always_include:
        return False
    
    # Skip common build/temp directories and files
    skip_patterns = {
        '__pycache__', '.git', '.vscode', '.pio', 'build', 
        'dist', 'node_modules', '.pytest_cache', 'CMakeFiles'
    }
    
    # Check if any part of the path contains skip patterns
    path_parts = set(file_path.parts)
    if path_parts.intersection(skip_patterns):
        return False
    
    # Include if extension matches or filename is in always_include
    return extension in include_extensions or filename in always_include

def get_file_category(file_path):
    """
    Categorize files for better organization in output
    """
    extension = file_path.suffix.lower()
    filename = file_path.name.lower()
    
    if filename == 'platformio.ini':
        return 'Configuration'
    elif extension in ['.h', '.hpp']:
        return 'Headers'
    elif extension in ['.cpp', '.c', '.ino']:
        return 'Source'
    elif extension in ['.md', '.txt']:
        return 'Documentation'
    elif extension in ['.json', '.xml', '.yaml', '.yml']:
        return 'Data'
    else:
        return 'Other'

def read_file_safely(file_path):
    """
    Safely read file content with encoding detection
    """
    encodings = ['utf-8', 'latin1', 'cp1252', 'iso-8859-1']
    
    for encoding in encodings:
        try:
            with open(file_path, 'r', encoding=encoding) as f:
                return f.read()
        except UnicodeDecodeError:
            continue
        except Exception as e:
            return f"ERROR READING FILE: {str(e)}"
    
    return "ERROR: Could not decode file with any encoding"

def scan_project_files(project_root):
    """
    Recursively scan project directories for files
    """
    project_path = Path(project_root)
    
    # Directories to scan
    scan_dirs = ['include', 'src']
    files_to_process = []
    
    # Add platformio.ini from root if exists
    platformio_ini = project_path / 'platformio.ini'
    if platformio_ini.exists():
        files_to_process.append(platformio_ini)
    
    # Scan specified directories recursively
    for dir_name in scan_dirs:
        dir_path = project_path / dir_name
        if dir_path.exists():
            # Recursively find all files
            for file_path in dir_path.rglob('*'):
                if file_path.is_file() and should_include_file(file_path):
                    files_to_process.append(file_path)
        else:
            print(f"Warning: Directory '{dir_name}' not found in project root")
    
    return files_to_process

def generate_file_tree(project_root, files_list):
    """
    Generate a visual file tree representation
    """
    project_path = Path(project_root)
    
    tree_lines = [f"📁 {project_path.name}/"]
    
    # Group files by directory
    dirs = {}
    for file_path in files_list:
        relative_path = file_path.relative_to(project_path)
        dir_path = relative_path.parent
        
        if dir_path not in dirs:
            dirs[dir_path] = []
        dirs[dir_path].append(relative_path.name)
    
    # Sort directories and files
    sorted_dirs = sorted(dirs.keys())
    
    for i, dir_path in enumerate(sorted_dirs):
        if str(dir_path) == '.':
            # Root files
            for filename in sorted(dirs[dir_path]):
                tree_lines.append(f"├── {filename}")
        else:
            # Directory files
            tree_lines.append(f"├── 📁 {dir_path}/")
            for j, filename in enumerate(sorted(dirs[dir_path])):
                prefix = "│   ├── " if j < len(dirs[dir_path]) - 1 else "│   └── "
                tree_lines.append(f"{prefix}{filename}")
    
    return "\n".join(tree_lines)

def concatenate_project_files(project_root, output_file='project_concatenated.txt'):
    """
    Main function to concatenate all project files
    """
    project_path = Path(project_root)
    
    if not project_path.exists():
        print(f"Error: Project directory '{project_root}' does not exist")
        return False
    
    # Scan for files
    print("🔍 Scanning project files...")
    files_to_process = scan_project_files(project_root)
    
    if not files_to_process:
        print("⚠️  No files found to process")
        return False
    
    print(f"📄 Found {len(files_to_process)} files to process")
    
    # Sort files by category and name for better organization
    files_to_process.sort(key=lambda f: (get_file_category(f), str(f)))
    
    # Generate output
    output_path = Path(output_file)
    
    try:
        with open(output_path, 'w', encoding='utf-8') as out_file:
            # Write header
            out_file.write("=" * 80 + "\n")
            out_file.write("COTOMETER PROJECT - CONCATENATED FILES\n")
            out_file.write("=" * 80 + "\n")
            out_file.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            out_file.write(f"Project Root: {project_path.absolute()}\n")
            out_file.write(f"Total Files: {len(files_to_process)}\n")
            out_file.write("=" * 80 + "\n\n")
            
            # Write file tree
            out_file.write("📋 PROJECT STRUCTURE\n")
            out_file.write("-" * 40 + "\n")
            out_file.write(generate_file_tree(project_root, files_to_process))
            out_file.write("\n\n")
            
            # Process each file
            current_category = None
            
            for i, file_path in enumerate(files_to_process, 1):
                relative_path = file_path.relative_to(project_path)
                category = get_file_category(file_path)
                
                # Add category header if changed
                if category != current_category:
                    current_category = category
                    out_file.write("=" * 80 + "\n")
                    out_file.write(f"📂 {category.upper()} FILES\n")
                    out_file.write("=" * 80 + "\n\n")
                
                # File header
                out_file.write("┌" + "─" * 78 + "┐\n")
                out_file.write(f"│ 📄 FILE {i:3d}/{len(files_to_process)}: {str(relative_path):<65} │\n")
                out_file.write(f"│ 📁 Path: {str(file_path.parent):<66} │\n")
                out_file.write(f"│ 📊 Size: {file_path.stat().st_size:,} bytes{'':<50} │\n")
                out_file.write("└" + "─" * 78 + "┘\n\n")
                
                # File content
                if file_path.stat().st_size == 0:
                    out_file.write("🗂️  [EMPTY FILE]\n\n")
                else:
                    content = read_file_safely(file_path)
                    out_file.write(content)
                    
                    # Ensure file ends with newline
                    if not content.endswith('\n'):
                        out_file.write('\n')
                
                out_file.write("\n" + "─" * 80 + "\n\n")
            
            # Write footer
            out_file.write("=" * 80 + "\n")
            out_file.write("END OF CONCATENATED PROJECT FILES\n")
            out_file.write("=" * 80 + "\n")
            out_file.write(f"Total files processed: {len(files_to_process)}\n")
            out_file.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
    
    except Exception as e:
        print(f"❌ Error writing output file: {str(e)}")
        return False
    
    print(f"✅ Successfully created '{output_file}'")
    print(f"📊 File size: {output_path.stat().st_size:,} bytes")
    print(f"📄 Files processed: {len(files_to_process)}")
    
    return True

def main():
    """
    Command line interface
    """
    if len(sys.argv) < 2:
        print("Usage: python concatenate_project.py <project_directory> [output_file]")
        print("Example: python concatenate_project.py ./CoToMeter-Firmware")
        print("Example: python concatenate_project.py ./CoToMeter-Firmware my_project.txt")
        sys.exit(1)
    
    project_dir = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'cotometer_project_concatenated.txt'
    
    success = concatenate_project_files(project_dir, output_file)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()