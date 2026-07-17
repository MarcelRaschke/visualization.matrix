#!/bin/bash
# Release script for visualization.matrix
# Usage: ./scripts/release.sh [version] [--dry-run]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if version is provided
if [ -z "$1" ]; then
    echo -e "${RED}Error: No version provided.${NC}"
    echo "Usage: $0 <version> [--dry-run]"
    echo "Example: $0 1.1.0"
    echo "Example: $0 1.1.0 --dry-run"
    exit 1
fi

VERSION=$1
DRY_RUN=false

# Check for dry-run flag
if [ "$2" = "--dry-run" ]; then
    DRY_RUN=true
    echo -e "${YELLOW}Running in dry-run mode. No changes will be committed.${NC}"
fi

# Get current branch
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

# Check if we're on master or Matrix branch
if [[ "$CURRENT_BRANCH" != "master" && "$CURRENT_BRANCH" != "Matrix" && "$CURRENT_BRANCH" != "Nexus" ]]; then
    echo -e "${RED}Error: Releases must be created from master, Matrix, or Nexus branch.${NC}"
    echo "Current branch: $CURRENT_BRANCH"
    exit 1
fi

# Validate version format (X.Y.Z)
if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo -e "${RED}Error: Version must be in format X.Y.Z (e.g., 1.1.0).${NC}"
    exit 1
fi

echo -e "${GREEN}Preparing release v$VERSION${NC}"

# Update version in addon.xml.in if it exists
if [ -f "addon.xml.in" ]; then
    echo -e "${YELLOW}Updating version in addon.xml.in to v$VERSION${NC}"
    if [ "$DRY_RUN" = true ]; then
        echo "  [DRY RUN] Would update addon.xml.in"
    else
        sed -i "s/@PROJECT_VERSION@/$VERSION/g" addon.xml.in
        git add addon.xml.in
    fi
fi

# Update CHANGELOG.md
if [ -f "CHANGELOG.md" ]; then
    echo -e "${YELLOW}Updating CHANGELOG.md for v$VERSION${NC}"
    if [ "$DRY_RUN" = true ]; then
        echo "  [DRY RUN] Would update CHANGELOG.md"
    else
        # Get current date
        DATE=$(date +"%Y-%m-%d")
        
        # Get latest tag
        LATEST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")
        
        # Generate changelog entries
        echo "" >> CHANGELOG.md
        echo "## [$VERSION] - $DATE" >> CHANGELOG.md
        echo "" >> CHANGELOG.md
        echo "### Added" >> CHANGELOG.md
        echo "" >> CHANGELOG.md
        echo "### Changed" >> CHANGELOG.md
        echo "" >> CHANGELOG.md
        echo "### Fixed" >> CHANGELOG.md
        echo "" >> CHANGELOG.md
        
        # Get commits since last tag
        if [ "$LATEST_TAG" != "0.0.0" ]; then
            echo "### Commits" >> CHANGELOG.md
            echo "" >> CHANGELOG.md
            git log $LATEST_TAG..HEAD --pretty=format:"- %s (%h) - %an" --reverse >> CHANGELOG.md
            echo "" >> CHANGELOG.md
        fi
        
        git add CHANGELOG.md
    fi
fi

# Commit changes
if [ "$DRY_RUN" = false ]; then
    echo -e "${YELLOW}Committing changes${NC}"
    git commit -m "release: prepare for v$VERSION"
    
    # Create tag
    echo -e "${YELLOW}Creating tag v$VERSION${NC}"
    git tag -a "v$VERSION" -m "Release v$VERSION"
    
    echo -e "${GREEN}Release v$VERSION prepared successfully!${NC}"
    echo ""
    echo "To publish the release, run:"
    echo "  git push origin $CURRENT_BRANCH"
    echo "  git push origin v$VERSION"
else
    echo -e "${GREEN}Dry run completed successfully!${NC}"
    echo ""
    echo "To create the release, run without --dry-run:"
    echo "  ./scripts/release.sh $VERSION"
fi
