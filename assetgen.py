import os
from pathlib import Path

def sanitize_name(name: str) -> str:
    """Convert a filename to a valid C++ identifier."""
    return name.replace(" ", "_").replace("-", "_").replace(".", "_")

def generate_resource_files(directory: str, header_file: str, source_file: str):
    directory = Path(directory)
    if not directory.is_dir():
        raise NotADirectoryError(f"Invalid directory: {directory}")

    # Collect all files from subdirectories with their subdir context
    all_files = []
    for subdir in directory.iterdir():
        if subdir.is_dir():
            # Special handling for 'models' directory
            if subdir.name == 'models':
                # Each subfolder in models becomes a single asset pointing to scene.bin
                for model_subdir in subdir.iterdir():
                    if model_subdir.is_dir():
                        scene_bin_path = model_subdir / "scene.gltf"
                        if scene_bin_path.exists():
                            all_files.append(('models', model_subdir.name, scene_bin_path))
                        else:
                            print(f"Warning: scene.gltf not found in {model_subdir}")
            else:
                # Original behavior for other directories
                files = [f for f in subdir.iterdir() if f.is_file()]
                for file_path in files:
                    all_files.append((subdir.name, file_path.stem, file_path))

    if not all_files:
        raise ValueError("No files found in any subdirectory")

    # Generate header file (.hpp)
    with open(header_file, "w") as f:
        # Header guard
        f.write("/*THIS FILE IS AUTO GENERATED*/\n")
        f.write("#ifndef RESOURCE_IDS_HPP\n")
        f.write("#define RESOURCE_IDS_HPP\n\n")
        f.write("#include <cstdint>\n\n")

        # Single namespace resources
        f.write("namespace resources {\n\n")

        # Generate single enum with all assets
        f.write("enum AssetId : unsigned int{\n")
        f.write("    None = 0,\n")

        # Generate enum entries for all files
        for i, file_info in enumerate(all_files, 1):
            if len(file_info) == 3:  # (subdir_name, identifier, file_path)
                subdir_name, identifier, _ = file_info
                enum_name = f"{subdir_name.capitalize()}_{sanitize_name(identifier)}"
            else:  # Legacy format support
                subdir_name, file_path = file_info
                enum_name = f"{subdir_name.capitalize()}_{sanitize_name(file_path.stem)}"
            f.write(f"    {enum_name} = {i},\n")

        f.write("};\n\n")

        # Function declaration only
        f.write("const char* path(AssetId id);\n\n")

        # Close namespace
        f.write("} // namespace resources\n\n")
        f.write("#endif // RESOURCE_IDS_HPP\n")

    # Generate source file (.cpp)
    with open(source_file, "w") as f:
        f.write("/*THIS FILE IS AUTO GENERATED*/\n")
        f.write('#include "resource_ids.hpp"\n\n')

        # Function implementation
        f.write("const char* resources::path(AssetId id) {\n")
        f.write("    switch (id) {\n")

        for i, file_info in enumerate(all_files, 1):
            if len(file_info) == 3:  # (subdir_name, identifier, file_path)
                subdir_name, identifier, file_path = file_info
                enum_name = f"{subdir_name.capitalize()}_{sanitize_name(identifier)}"
            else:  # Legacy format support
                subdir_name, file_path = file_info
                enum_name = f"{subdir_name.capitalize()}_{sanitize_name(file_path.stem)}"

            relative_path = file_path.relative_to(directory.parent).as_posix()
            f.write(f"        case AssetId::{enum_name}:\n")
            f.write(f'            return "{relative_path}";\n')

        f.write("        default:\n")
        f.write('            return nullptr;\n')
        f.write("    }\n")
        f.write("}\n")

if __name__ == "__main__":
    try:
        generate_resource_files("resources", "resource_ids.hpp", "resource_ids.cpp")
        print("Generated resource_ids.hpp and resource_ids.cpp successfully!")
    except (NotADirectoryError, ValueError) as e:
        print(f"Error: {e}")
