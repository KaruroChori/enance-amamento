This program is used internally by the library to:
- Generate documentation for the library of supported SDF, including the demo images.
- Generate schemas for the different SDF primitives and operators, so that they can be used to compose and validate proper XML files.

It operates in two steps:
- First, the script helper `helper.ts` is run. This will generate the full list of operators and primitives based on the current filesystem.
  Right now it is implemented in typescript for simplicity, and it is not something which must be run at each commit or by anyone.  
- Second, the main application is compiled, using the headers provided by `helper.ts`

At this point `gen-schemas` can run, and will automatically populate content in several locations of the repository.  
They are not tracked in git, but they are part of the release distribution.

## Generated files

Markdown documentation will be generated in `/docs/library`. `mkdocs` will be responsible for its final HTML as for every other markdown file.   
XML Schemas will be generated in `/dist/root-website` so that they are distributed as part of the github pages themselves.