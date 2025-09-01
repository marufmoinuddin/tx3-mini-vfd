#!/bin/bash
# setup_remote.sh - Helper script to set up git remote

echo "TX3 Mini VFD Controller - Git Setup Helper"
echo "=========================================="
echo ""

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "Error: Not in a git repository!"
    exit 1
fi

echo "Current repository status:"
git log --oneline -1
echo ""

# Prompt for repository URL
echo "Please enter your remote repository URL:"
echo "Examples:"
echo "  GitHub: https://github.com/marufmoinuddin/tx3-mini-vfd.git"
echo "  GitLab: https://gitlab.com/marufmoinuddin/tx3-mini-vfd.git"
echo "  SSH:    git@github.com:marufmoinuddin/tx3-mini-vfd.git"
echo ""
read -p "Repository URL: " repo_url

if [ -z "$repo_url" ]; then
    echo "No URL provided. Exiting."
    exit 1
fi

# Add remote origin
echo ""
echo "Adding remote origin..."
git remote add origin "$repo_url"

if [ $? -eq 0 ]; then
    echo "Remote origin added successfully!"
    echo ""
    echo "Pushing to remote repository..."
    git push -u origin main
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Repository successfully pushed to: $repo_url"
        echo ""
        echo "You can now:"
        echo "- View your code online at the repository URL"
        echo "- Clone it on other machines with: git clone $repo_url"
        echo "- Make future pushes with: git push"
    else
        echo "❌ Error pushing to remote repository."
        echo "Please check your credentials and repository URL."
    fi
else
    echo "❌ Error adding remote origin."
    echo "Please check the repository URL."
fi
