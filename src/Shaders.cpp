#include "Shaders.h"
bool initialized = false;

std::string int2str(const int n)
{
	std::ostringstream stm;
	stm << n;
	return stm.str();
}

ComputeShader::ComputeShader()
{

}

ComputeShader::ComputeShader(const std::string file_path)
{
	LoadShader(file_path);
}

std::string ComputeShader::LoadFileText(fs::path path)
{
	std::string text;
	std::ifstream TextStream(path, std::ios::in);
	if (TextStream.is_open())
	{
		std::string Line = "";
		while (getline(TextStream, Line))
			text += Line + "\n";
		TextStream.close();
	}
	else
	{
		ERROR_MSG("Impossible to open text file");
	}
	return text;
}

void ComputeShader::Delete()
{
	glDeleteProgram(ProgramID);
}


void ComputeShader::LoadShader(const std::string file_path)
{
		// Create the shaders
		GLuint ComputeShaderID = glCreateShader(GL_COMPUTE_SHADER);

		// Read the Compute Shader code from the file
		std::string ComputeShaderCode = PreprocessIncludes(fs::path(file_path));

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Compute Shader
		char const * ComputeSourcePointer = ComputeShaderCode.c_str();
		glShaderSource(ComputeShaderID, 1, &ComputeSourcePointer, NULL);
		glCompileShader(ComputeShaderID);

		// Check Compute Shader
		glGetShaderiv(ComputeShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(ComputeShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (Result == GL_FALSE)
		{
			std::vector<char> ComputeShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(ComputeShaderID, InfoLogLength, NULL, &ComputeShaderErrorMessage[0]);
			ERROR_MSG(("Compute shader compilation error. \n" + std::string(&ComputeShaderErrorMessage[0])).c_str());
			SaveErrors(file_path, ComputeShaderCode, std::string(&ComputeShaderErrorMessage[0]));
		}

		// Link the program
		ProgramID = glCreateProgram();
		glAttachShader(ProgramID, ComputeShaderID);
		glLinkProgram(ProgramID); 

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (Result == GL_FALSE)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			ERROR_MSG(("Compute program error. \n" + std::string(&ProgramErrorMessage[0])).c_str());
		}

		glDetachShader(ProgramID, ComputeShaderID);

		glDeleteShader(ComputeShaderID);
}

void ComputeShader::Run(vec2 global)
{
	glUseProgram(ProgramID);
	glDispatchCompute(ceil(global.x), ceil(global.y), 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ComputeShader::setUniform(std::string name, float X, float Y)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform2f(A, X, Y);
}

void ComputeShader::setUniform(std::string name, float X, float Y, float Z)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform3f(A, X, Y, Z);
}

void ComputeShader::setUniform(std::string name, float X)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform1f(A, X);
}

void ComputeShader::setUniform(std::string name, int X)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform1i(A, X);
}

void ComputeShader::setUniform(std::string name, glm::mat3 X, bool transpose)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniformMatrix3fv(A, 1, transpose, glm::value_ptr(X));
}

void ComputeShader::setUniform(std::string name, glm::vec3 X)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform3fv(A, 1, glm::value_ptr(X));
}

void ComputeShader::setUniform(std::string name, glm::vec2 X)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform2fv(A, 1, glm::value_ptr(X));
}

void ComputeShader::setUniform(int i, GLuint tid)
{
	glUseProgram(ProgramID);
	GLuint A = glGetUniformLocation(ProgramID, ("iTexture" + int2str(i)).c_str());
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, tid);
	glUniform1i(A, i);
}

void ComputeShader::setCameraObj(std::string name, gl_camera cam)
{
	setUniform(name + ".position", cam.position);
	setUniform(name + ".bokeh", cam.bokeh);
	setUniform(name + ".dirx", cam.dirx);
	setUniform(name + ".diry", cam.diry);
	setUniform(name + ".dirz", cam.dirz);
	setUniform(name + ".aspect_ratio", cam.aspect_ratio);
	setUniform(name + ".exposure", cam.exposure);
	setUniform(name + ".focus", cam.focus);
	setUniform(name + ".FOV", cam.FOV);
	setUniform(name + ".mblur", cam.mblur);
	setUniform(name + ".position", cam.position);
	setUniform(name + ".resolution", cam.resolution);
	setUniform(name + ".size", cam.size);
	setUniform(name + ".bloomradius", cam.bloomradius);
	setUniform(name + ".bloomintensity", cam.bloomintensity);
	setUniform(name + ".speckle", cam.speckle);
	setUniform(name + ".cross_eye", cam.cross_eye);
	setUniform(name + ".eye_separation", cam.eye_separation);
	setUniform("iFrame", cam.iFrame);
}

void ComputeShader::setCamera(gl_camera cam)
{
	setCameraObj("Camera", cam);
	setCameraObj("PrevCamera", prev_camera);
	prev_camera = cam;
}

GLuint ComputeShader::getNativeHandle()
{
	return ProgramID;
}

bool INIT()
{
	if (initialized)
	{
		return true;
	}
	if ( glewInit() != GLEW_OK) 
	{
		ERROR_MSG("Failed to initialize GLEW\n");
		return false;
	}
	initialized = true;
	return true;
}


std::string ComputeShader::PreprocessIncludes(const fs::path& filename, int level /*= 0 */)
{
	if (level > 32)
		ERROR_MSG("Header inclusion depth limit reached, might be caused by cyclic header inclusion");
	using namespace std;

	//match regular expression
	static const regex re("^[ ]*#include\s*[\"<](.*)[\">].*");
	stringstream input;
	stringstream output;
	input << LoadFileText(filename);

	smatch matches;
	string line;
	while (std::getline(input, line))
	{
		if (regex_search(line, matches, re))
		{
			//add the code from the included file
			std::string include_file = compute_folder + "/" + matches[1].str();
			output << PreprocessIncludes(include_file, level + 1) << endl;
		}
		else
		{
			output << line << "\n";
		}
	}
	return output.str();
}

void ComputeShader::SaveErrors(const fs::path& filename, std::string code, std::string errors)
{
	fs::path outf = (filename.parent_path() / filename.filename()).concat("_error.txt");
	std::ofstream error_out(outf);
	
	error_out << code << std::endl << errors;
	error_out.close();
}