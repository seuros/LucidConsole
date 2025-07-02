#!/usr/bin/env python3
"""
Update version in main.c from VERSION.txt
This script is run during build to sync version numbers
"""

import os
import re
import sys

def update_version():
    # Get the project root directory
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    version_file = os.path.join(project_root, 'VERSION.txt')
    main_file = os.path.join(project_root, 'main', 'main.c')
    
    # Read version from VERSION.txt
    try:
        with open(version_file, 'r') as f:
            version = f.read().strip()
    except FileNotFoundError:
        print(f"Error: {version_file} not found")
        return False
    
    if not version:
        print("Error: VERSION.txt is empty")
        return False
    
    print(f"Updating version to: {version}")
    
    # Read main.c
    try:
        with open(main_file, 'r') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: {main_file} not found")
        return False
    
    # Replace version in main.c
    pattern = r'#define LUCID_VERSION "[^"]*"'
    replacement = f'#define LUCID_VERSION "{version}"'
    
    new_content = re.sub(pattern, replacement, content)
    
    if new_content == content:
        print("Error: Could not find LUCID_VERSION definition in main.c")
        return False
    
    # Write updated main.c
    try:
        with open(main_file, 'w') as f:
            f.write(new_content)
        print(f"Successfully updated {main_file}")
        return True
    except Exception as e:
        print(f"Error writing {main_file}: {e}")
        return False

if __name__ == '__main__':
    success = update_version()
    sys.exit(0 if success else 1)