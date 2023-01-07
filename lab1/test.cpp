/* draw the sun
    glUseProgram(shaderProgramID);
    generateObjectBufferMesh(mesh_data3);
    glm::mat4 Translation4(1.0f, 0.0f, 0.0f, -10.0f,
                           0.0f, 1.0f, 0.0f, 10.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 model4      = glm::mat4(3.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &persp_proj[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model4[0][0]);
    glUniformMatrix4fv(gTranslationLocation, 1, GL_TRUE, &Translation4[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, mesh_data3.mPointCount);*/
    