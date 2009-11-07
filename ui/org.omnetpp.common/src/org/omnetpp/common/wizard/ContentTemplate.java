/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.
  
  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.common.wizard;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;

import org.apache.commons.collections.CollectionUtils;
import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.graphics.Image;
import org.omnetpp.common.CommonPlugin;
import org.omnetpp.common.util.LicenseUtils;
import org.omnetpp.common.util.StringUtils;
import org.omnetpp.common.wizard.support.FileUtils;
import org.omnetpp.common.wizard.support.IDEUtils;
import org.omnetpp.common.wizard.support.LangUtils;

import freemarker.cache.StringTemplateLoader;
import freemarker.ext.beans.BeansWrapper;
import freemarker.template.Configuration;
import freemarker.template.Template;
import freemarker.template.TemplateException;

/**
 * The default implementation of IContentTemplate, using FreeMarker templates.
 * The createCustomPages() method is left undefined, but subclasses could use 
 * XSWTWizardPage for implementing it.
 * 
 * @author Andras
 */
public abstract class ContentTemplate implements IContentTemplate {
	public static final Image DEFAULT_ICON = CommonPlugin.getImage("icons/obj16/wiztemplate.png");
	
	private static final String SETOUTPUT_MARKER = "@@@@ setoutput \"${file}\" @@@@\n";
	private static final String SETOUTPUT_PATTERN = "(?s)@@@@ setoutput \"(.*?)\" @@@@\n"; // filename must be $1
	 
	// template attributes
    private String name;
    private String description;
    private String category;
    private Image image = DEFAULT_ICON;

    // we need this ClassLoader for the Class.forName() calls in for both FreeMarker and XSWT.
    // Without it, templates (BeanWrapper) and XSWT would only be able to access classes 
    // from the eclipse plug-in from which their code was loaded. In contrast, we want them to
    // have access to classes defined in this plug-in (FileUtils, IDEUtils, etc), and also
    // to the contents of jars loaded at runtime from the user's projects in the workspace.
    // See e.g. freemarker.template.utility.ClassUtil.forName(String)
    private ClassLoader classLoader = null;  
    
    // FreeMarker configuration (stateless)
    private Configuration freemarkerConfiguration = null;
    
    public ContentTemplate() {
    }

    public ContentTemplate(String name, String category, String description) {
        super();
        this.name = name;
        this.category = category;
        this.description = description;
    }

    public ContentTemplate(String name, String category, String description, Image image) {
    	this(name, category, description);
    	this.image = image;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
    	this.name = name;
    }
    
    public String getCategory() {
    	return category;
    }
    
    public void setCategory(String category) {
    	this.category = category;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
    	this.description = description;
    }
    
    public Image getImage() {
        return image;
    }

    public void setImage(Image image) {
    	this.image = image;
    }
    
    public CreationContext createContext(IContainer folder) {
    	CreationContext context = new CreationContext(folder);
    	
    	// pre-register some potentially useful template variables
    	Map<String, Object> variables = context.getVariables();
        variables.put("templateName", name);
        variables.put("templateDescription", description);
        variables.put("templateCategory", category);
		variables.put("rawProjectName", folder.getFullPath().toString());
		variables.put("projectName", folder.getFullPath().toString());
		variables.put("ProjectName", StringUtils.capitalize(StringUtils.makeValidIdentifier(folder.getName())));
		variables.put("projectname", StringUtils.lowerCase(StringUtils.makeValidIdentifier(folder.getName())));
		variables.put("PROJECTNAME", StringUtils.upperCase(StringUtils.makeValidIdentifier(folder.getName())));
        Calendar cal = Calendar.getInstance();
        variables.put("date", cal.get(Calendar.YEAR)+"-"+cal.get(Calendar.MONTH)+"-"+cal.get(Calendar.DAY_OF_MONTH));
        variables.put("year", ""+cal.get(Calendar.YEAR));
        variables.put("author", System.getProperty("user.name"));
        String licenseCode = LicenseUtils.getDefaultLicense();
        String licenseProperty = licenseCode.equals(LicenseUtils.NONE) ? "" : licenseCode; // @license(custom) is needed, so that wizards knows whether to create banners
        variables.put("licenseCode", licenseProperty); // for @license in package.ned
        variables.put("licenseText", LicenseUtils.getLicenseNotice(licenseCode));
        variables.put("bannerComment", LicenseUtils.getBannerComment(licenseCode, "//"));

        return context;
    }
    
    public ClassLoader getClassLoader() {
        if (classLoader == null)
            classLoader = createClassLoader();
        return classLoader;
    }

    protected ClassLoader createClassLoader() {
        return getClass().getClassLoader();
    }

    protected Configuration getFreemarkerConfiguration() {
        if (freemarkerConfiguration == null)
            freemarkerConfiguration = createFreemarkerConfiguration();
        return freemarkerConfiguration;
    }
    
    protected Configuration createFreemarkerConfiguration() {
        // note: subclasses should override to add a real template loader
        Configuration cfg = new Configuration();
        
        cfg.setNumberFormat("computer"); // prevent digit grouping with comma

        // add loader for built-in macro definitions
        final String BUILTINS = "[builtins]";
        cfg.addAutoInclude(BUILTINS);
        StringTemplateLoader loader = new StringTemplateLoader();
        loader.putTemplate(BUILTINS, 
                "<#macro setoutput file>\n" + 
                SETOUTPUT_MARKER + 
        		"</#macro>\n");
        cfg.setTemplateLoader(loader);

        return cfg;
    }
    
	/**
     * Resolves references to other variables. Should be called from doPerformFinish() before
     * actually processing the templates.
     * 
     * Some variables contain references to other variables (e.g. 
     * "org.example.${projectname}"); substitute them.
     */
	protected void substituteNestedVariables(CreationContext context) throws CoreException {
		Map<String, Object> variables = context.getVariables();
        try {
        	for (String key : variables.keySet()) {
        		Object value = variables.get(key);
        		if (value instanceof String) {
        			String newValue = evaluate((String)value, variables);
        			variables.put(key, newValue);
        		}
        	}
        } catch (Exception e) {
        	throw CommonPlugin.wrapIntoCoreException(e);
        }
	}

	/** 
	 * Performs template processing on the given string, and returns the result.
	 */
	public String evaluate(String value, Map<String, Object> variables) throws TemplateException {
        // classLoader stuff -- see freemarker.template.utility.ClassUtil.forName(String)
	    ClassLoader oldClassLoader = Thread.currentThread().getContextClassLoader();
        Thread.currentThread().setContextClassLoader(getClassLoader());
	    
		try {
		    Configuration cfg = getFreemarkerConfiguration();
			int k = 0;
			while (true) {
				Template template = new Template("text" + k++, new StringReader(value), cfg, "utf8");
				StringWriter writer = new StringWriter();
				template.process(variables, writer);
				String newValue = writer.toString();
				if (value.equals(newValue))
					return value;
				value = newValue;
			}
		} 
		catch (IOException e) {
			CommonPlugin.logError(e); 
			return ""; // cannot happen, we work with string reader/writer only
		}
		finally {
	        Thread.currentThread().setContextClassLoader(oldClassLoader);
		}
	}

    /**
     * Utility method for doConfigure. Copies a resource into the project,  
     * performing variable substitutions in it. If the template contained 
     * &lt;@setoutput file="..."&gt; tags, multiple files will be saved. 
     * 
     * See also suppressIfBlank().
     * @param suppressIfBlank 
     */
    protected void createFile(IFile file, Configuration freemarkerConfig, String templateName, CreationContext context) throws CoreException {
        // classLoader stuff -- see freemarker.template.utility.ClassUtil.forName(String)
        ClassLoader oldClassLoader = Thread.currentThread().getContextClassLoader();
        Thread.currentThread().setContextClassLoader(getClassLoader());

        // substitute variables
    	String content;    	
        try {
        	// make Math, FileUtils, StringUtils and static methods of other classes available to the template
        	// see chapter "Bean wrapper" in the FreeMarker manual 
        	context.getVariables().put("Math", BeansWrapper.getDefaultInstance().getStaticModels().get(Math.class.getName()));
        	context.getVariables().put("FileUtils", BeansWrapper.getDefaultInstance().getStaticModels().get(FileUtils.class.getName()));
        	context.getVariables().put("StringUtils", BeansWrapper.getDefaultInstance().getStaticModels().get(StringUtils.class.getName()));
        	context.getVariables().put("CollectionUtils", BeansWrapper.getDefaultInstance().getStaticModels().get(CollectionUtils.class.getName()));
        	context.getVariables().put("IDEUtils", BeansWrapper.getDefaultInstance().getStaticModels().get(IDEUtils.class.getName()));
        	context.getVariables().put("LangUtils", BeansWrapper.getDefaultInstance().getStaticModels().get(LangUtils.class.getName()));

        	context.getVariables().put("classes", BeansWrapper.getDefaultInstance().getStaticModels());
        	
        	// perform template substitution
			Template template = freemarkerConfig.getTemplate(templateName, "utf8");
			StringWriter writer = new StringWriter();
			template.process(context.getVariables(), writer);
			content = writer.toString();
		} catch (Exception e) {
			throw CommonPlugin.wrapIntoCoreException(e);
		}
        finally {
            Thread.currentThread().setContextClassLoader(oldClassLoader);
        }

		// normalize line endings, remove multiple blank lines, etc
		content = content.replace("\r\n", "\n");
		content = content.replaceAll("\n\n\n+", "\n\n");
		content = content.trim() + "\n";

		// implement <@setoutput file="fname"/> tag: split content to files. "" means the main file
        List<String> chunks = StringUtils.splitPreservingSeparators(content, Pattern.compile(SETOUTPUT_PATTERN));
        Map<String, String> fileContent = new HashMap<String, String>(); // fileName -> content
        fileContent.put("", chunks.get(0));
        for (int i = 1; i < chunks.size(); i += 2) {
            String fileName = chunks.get(i).replaceAll(SETOUTPUT_PATTERN, "$1");
            String chunk = chunks.get(i+1);
            if (!fileContent.containsKey(fileName))
                fileContent.put(fileName, chunk);
            else 
                fileContent.put(fileName, fileContent.get(fileName) + chunk);  // append
        }
		
		// save files (unless blank)
        for (String fileName : fileContent.keySet()) {
            IFile fileToSave = fileName.equals("") ? file : file.getParent().getFile(new Path(fileName));
            String contentToSave = fileContent.get(fileName);
            if (!StringUtils.isBlank(contentToSave)) {
                // save the file if not blank. Note: we do NOT delete the existing file with 
                // the same name if contentToSave is blank; this is documented behavior.
                byte[] bytes = contentToSave.getBytes(); 
                if (!fileToSave.exists())
                    fileToSave.create(new ByteArrayInputStream(bytes), true, context.getProgressMonitor());
                else
                    fileToSave.setContents(new ByteArrayInputStream(bytes), true, true, context.getProgressMonitor());
            }
        }
    }

    /**
     * Creates a folder, relative to the context resource (which must here be an IContainer). 
     * If the parent folder(s) do not exist, they are created.
     */
    protected void createFolder(String containerRelativePath, CreationContext context) throws CoreException {
        if (!(context.getFolder() instanceof IContainer))
            throw new IllegalStateException("createFolder() may only be used if the resource is an IContainer");

        IContainer container = (IContainer)context.getFolder();
        IFolder folder = container.getFolder(new Path(containerRelativePath));
        if (!folder.getParent().exists())
            createFolder(folder.getParent().getProjectRelativePath().toString(), context);
        if (!folder.exists())
            folder.create(true, true, context.getProgressMonitor());
    }
    
}
