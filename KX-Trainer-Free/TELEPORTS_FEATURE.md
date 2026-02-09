# Teleports Feature Documentation

## Overview
The Teleports feature allows you to save, organize, and quickly teleport to different locations in the game. Teleport points are organized into groups for better management.

## Features

### Group Management
- **Multiple Groups**: Organize teleport points into different groups (e.g., "Farming Spots", "World Bosses", etc.)
- **Add Group**: Create new groups to organize your teleports
- **Rename Group**: Change the name of existing groups
- **Delete Group**: Remove groups (with confirmation to prevent accidental deletion)
- **Group Dropdown**: Quickly switch between different groups

### Teleport Management
- **Add Teleport**: Quickly add a teleport at your current game position
  - Automatically captures current X, Y, Z coordinates
  - Generates unique names (Teleport 1, Teleport 2, etc.)
  - Map ID defaults to -1 (can be edited later)
  
- **Edit Teleport**: Modify existing teleport points
  - Change name
  - Manually edit X, Y, Z coordinates
  - Set current position button for easy coordinate capture
  - Edit map ID
  - Map name dropdown (will be available when map data JSON is provided)
  
- **Delete Teleport**: Remove teleport points with confirmation dialog

- **Teleport**: Click any teleport button to instantly move to that location

### Import/Export
- **JSON Format**: All teleports are stored in `config/teleports.json`
- **Import JSON**: Import teleport groups from JSON files (button present, file picker to be implemented)
- **Auto-save**: Changes are automatically saved to the JSON file

## JSON Structure

```json
{
  "teleport_groups": [
    {
      "name": "Group 1",
      "teleports": [
        {
          "name": "Teleport 1",
          "coordinates": [
            -17230.312,
            -21172.797,
            28625.05
          ],
          "map": 1195
        }
      ]
    }
  ]
}
```

### JSON Validation
When importing, the JSON must:
- Have a `teleport_groups` array at the root
- Each group must have `name` and `teleports` properties
- Each teleport must have `name`, `coordinates` array (3 floats), and `map` (integer)

## Usage

### Adding a Teleport
1. Navigate to the location in-game where you want to save a teleport
2. Click the **"Add Teleport"** button
3. A new teleport will be created with your current coordinates
4. Click the **Edit** button to customize the name and map ID

### Organizing with Groups
1. Click **"Add Group"** to create a new group
2. Enter a group name and click **"Add"**
3. Switch to the new group using the dropdown
4. Add teleports to the current group

### Editing a Teleport
1. Click the **"Edit"** button next to a teleport
2. Modify the name, coordinates, or map ID
3. Use **"Set Current Position"** to update coordinates to your current location
4. Click **"Save"** to apply changes

### Deleting
- **Delete Teleport**: Click the **"X"** button next to a teleport, then confirm
- **Delete Group**: Click **"Delete Group"**, then confirm (deletes all teleports in the group)

## Future Enhancements
- Map name dropdown populated from a map data JSON file
- File picker dialog for importing JSON files
- Export individual groups to JSON (already implemented in backend)
- Drag and drop to reorder teleports
- Color-coded map icons based on current/accessible maps

## Technical Details
- Configuration file: `config/teleports.json` (created automatically)
- Unique teleport names within each group
- Coordinates stored as floats (X, Y, Z)
- Map ID stored as integer
- Automatic persistence on all changes
