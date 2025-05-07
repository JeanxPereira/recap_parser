#!/bin/bash

# Fetch tags
git fetch --tags

# Get the latest tag or use a default version if no tag exists
current_version=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")

# Remove the 'v' prefix if it exists
current_version="${current_version#v}"

# Separate the string into an array
IFS='.' read -ra numbers <<< "$current_version"

# Ensure we have three numbers, defaulting to 0 if not present
major=${numbers[0]:-0}
minor=${numbers[1]:-0}
patch=${numbers[2]:-0}

# Increment the patch version
new_patch=$((patch + 1))

# Output the new version
new_version="${major}.${minor}.${new_patch}"
echo "$new_version"